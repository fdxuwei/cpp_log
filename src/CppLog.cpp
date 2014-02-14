#include <ctime>
#include <errno.h>
#include <iomanip>
#include <fstream>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include "CppLog.h"

#ifdef WIN32
	#include "zip.h"
	#define LOCAL_TIME(_tm, _tt) localtime_s(&_tm, &_tt)
#else
	#define LOCAL_TIME(_tm, _tt) localtime_r(&_tt, &_tm)
#endif

using namespace std;
using namespace boost::filesystem;

namespace CppLog
{
	// member functions for Log
	Log::Log()
		: m_LogLevel(LOG_LEVEL_FATAL)
	{}

	Log::~Log(){}

	AppenderList& Log::GetAppenderList()
	{
		return m_Appenders;
	}

	void Log::AddAppender(AppenderPtr appender)
	{
		m_Appenders.push_back(appender);
	}

	void Log::SetLogLevel(LOG_LEVEL level)
	{
		m_LogLevel = level;
	}

	LOG_LEVEL Log::GetLogLevel()
	{
		return m_LogLevel;
	}

	LogMutex& Log::GetMutex()
	{
		return m_Mutex;
	}
	
	// member functions for Appender


	// member functions for ConsoleAppender
	ConsoleAppender::ConsoleAppender(){}

	void ConsoleAppender::Write(const std::string& msg)
	{
		cout << msg;
	}

	ConsoleAppenderPtr ConsoleAppender::Create()
	{
		return ConsoleAppenderPtr(new ConsoleAppender());
	}

	// member functions for FileAppender
	FileAppender::FileAppender()
	{
		SetCompress(true);
		SetPrefixName("test");
	}

	FileAppender::~FileAppender()
	{
		m_filestream.close();
	}

	void FileAppender::Open()
	{
		ArrangeFiles();
		string sFileName = SynthesizeTodyFileName();
		m_filestream.open(sFileName.c_str(), ios_base::app);
		if(m_filestream.fail())
		{
			m_filestream.clear();
			cout << "open file failed: " << sFileName << endl;
		}
	}

	void FileAppender::Close()
	{
		m_filestream.close();
		if(m_filestream.fail())
		{
			m_filestream.clear();
			cout << "close file failed: " << endl;
		}
	}

	void FileAppender::Write(const std::string& msg)
	{
		Open();
		WriteWithoutFlush(msg);
		Close();
	}

	void FileAppender::WriteWithoutFlush(const std::string& msg)
	{
		m_filestream << msg;
	}

	FileAppenderPtr FileAppender::Create()
	{
		return FileAppenderPtr(new FileAppender());
	}

	// QueuedAppender
	QueuedFileAppender::QueuedFileAppender()
		: m_bRun (true)
	{
		m_ThreadPtr = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&QueuedFileAppender::Loop, this)));
	}

	QueuedFileAppender::~QueuedFileAppender()
	{
		m_bRun = false;
		m_ThreadPtr->join();
		Sync(); // flush all the messages in the queue before exit
	}

	void QueuedFileAppender::Sync()
	{
		std::string sMsg;
		FileAppender::Open();
		while(m_Queue.PopMsg(sMsg))
		{
			FileAppender::WriteWithoutFlush(sMsg);
		}
		FileAppender::Close();
	}

	void QueuedFileAppender::Loop()
	{
		while(m_bRun)
		{
			boost::this_thread::sleep(boost::posix_time::seconds(2));
			Sync();
		}
	}
	
	void QueuedFileAppender::Write(const std::string& msg)
	{
		m_Queue.PushMsg(msg);
	}

	QueuedFileAppenderPtr QueuedFileAppender::Create()
	{
		return QueuedFileAppenderPtr(new QueuedFileAppender());
	}

	// queue

	bool SafeQueue::PopMsg(std::string& sMsg)
	{
		boost::lock_guard<LogMutex> lg(m_QueueMutex);
		if(m_MsgQueue.empty())
		{
			return false;
		}
		sMsg = m_MsgQueue.front();
		m_MsgQueue.pop_front();
		return true;
	}

	void SafeQueue::PushMsg(const std::string& sMsg)
	{
		boost::lock_guard<LogMutex> lg(m_QueueMutex);
		m_MsgQueue.push_back(sMsg);
	}

	//utils
	string GetLogTime()
	{
		time_t ttNow;
		tm tmNow;
		std::stringstream stime;
		ttNow = time(NULL);
		LOCAL_TIME(tmNow, ttNow);
		stime << std::setfill('0') << std::setw(4) << tmNow.tm_year+1900 << "/";
		stime << std::setfill('0') << std::setw(2) << tmNow.tm_mon+1 << "/";
		stime << std::setfill('0') << std::setw(2) << tmNow.tm_mday << " ";
		stime << std::setfill('0') << std::setw(2) << tmNow.tm_hour << ":";
		stime << std::setfill('0') << std::setw(2) << tmNow.tm_min << ":";
		stime << std::setfill('0') << std::setw(2) << tmNow.tm_sec;
		return stime.str();
	}

	FileManager::FileManager()
		: m_sDir ("./")
	{
		SetCompress(true);
		SetMaxFileLife(100);
		SetPrefixName("test");
	}

	bool FileManager::SetDir(const std::string& sDir)
	{
		try
		{
			m_sDir = sDir;
			replace(m_sDir.begin(), m_sDir.end(), '\\', '/');
			if('/' != m_sDir[m_sDir.length()-1])
			{
				m_sDir += "/";
			}
			if(!boost::filesystem::exists(m_sDir))
			{
				if(!boost::filesystem::create_directory(m_sDir))
				{
					m_sDir = "./";
					return false;
				}
			}
		}
		catch(...)
		{
			return false;
		}
		return true;
	}

	string FileManager::FullPath(const std::string& sName)
	{
		return m_sDir + sName;
	}

	string FileManager::SynthesizeTodyFileName()
	{
		time_t ttNow = time(NULL);
		return FullPath(m_sPrefixName + "_" + GetDateString(ttNow) + ".log");
	}

	string FileManager::SynthesizeTodyFileStem()
	{
		time_t ttNow = time(NULL);
		return m_sPrefixName + "_" + GetDateString(ttNow);
	}

	string FileManager::SynthesizeEarlistFileStem()
	{
		time_t tt = time(NULL) - m_nMaxFileLife*24*3600;
		return m_sPrefixName + "_" + GetDateString(tt);
	}

	void FileManager::RemoveCompressedFile(const std::string &sStemName)
	{
#ifdef WIN32
		remove(FullPath(sStemName + ".zip"));
#else
		remove(FullPath(sStemName + ".gz"));
#endif
	}

	void FileManager::ArrangeFiles()
	{
		vector<string> vsLogFiles;
		vector<string> vsZipFiles;
		string sLogNameTody = SynthesizeTodyFileStem();   //今天日志名
		string sLogNameEarlist = SynthesizeEarlistFileStem();//最早有效日志名

		ListLogFileStem(vsLogFiles, vsZipFiles);

		for(vector<string>::iterator itZF = vsZipFiles.begin(); itZF != vsZipFiles.end(); itZF++)
		{
			if(*itZF < sLogNameEarlist)
			{
				// 删除
				try
				{
					RemoveCompressedFile(*itZF);
				}
				catch(filesystem_error e)
				{
					cout << e.what() << endl;;
				}
			}
		}

		for(vector<string>::iterator itLF = vsLogFiles.begin(); itLF != vsLogFiles.end(); itLF++)
		{
			if(m_bCompress &&  (*itLF < sLogNameTody) && (*itLF >= sLogNameEarlist))
			{
				Compress(*itLF);
			}
		}
	}

	void FileManager::Compress(const std::string &sStemName)
	{
		string sFullLogName = sStemName + ".log";
#ifdef WIN32
		string sFullZipName = sStemName + ".zip";
		
		HZIP hz = CreateZip(FullPath(sFullZipName).c_str(), 0);
		if(hz)
		{
			if(ZipAdd(hz, sFullLogName.c_str(), FullPath(sFullLogName).c_str()) == ZR_OK)
			{
				//删除对应log文件
				try
				{
					remove(FullPath(sFullLogName).c_str());;
				}
				catch(filesystem_error e)
				{
					cout << e.what() << endl;;
				}
			}
			CloseZip(hz);
		}
#else
		string cmd = "gzip " + FullPath(sFullLogName);
		system(cmd.c_str());
#endif
	}

	void FileManager::ListLogFileStem(vector<string> &vsLogFileStem, vector<string> &vsZipFileStem)
	{
		vsLogFileStem.clear();
		vsZipFileStem.clear();

		path p (m_sDir);   // p reads clearer than argv[1] in the following code

		try
		{
			if (exists(p))    // does p actually exist?
			{
				if (is_directory(p))      // is p a directory?
				{
					//				cout << p << " is a directory containing:\n";

					typedef vector<path> vec;             // store paths,
					vec v;                                // so we can sort them later

					copy(directory_iterator(p), directory_iterator(), back_inserter(v));

					sort(v.begin(), v.end());             // sort, since directory iteration
					// is not ordered on some file systems

					for (vec::const_iterator it (v.begin()); it != v.end(); ++it)
					{	
						string sExt = it->extension().string();
						string sStem = it->stem().string();

						if(sStem.find(m_sPrefixName) != string::npos)
						{
							if(sExt == ".log")
							{
								vsLogFileStem.push_back(sStem);
							}
							else if((sExt == ".zip") || sExt == ".gz")
							{
								vsZipFileStem.push_back(sStem);
							}
						}
					}
				}
			}
		}
		catch (const filesystem_error& )
		{
			//cout << ex.what() << '\n';
		}

	}

	string FileManager::GetDateString(time_t tt)
	{
		tm _tm;
		LOCAL_TIME(_tm, tt);
		char buf[10];
		sprintf(buf, "%04d%02d%02d", _tm.tm_year+1900, _tm.tm_mon+1, _tm.tm_mday);
		return buf;
	}
}
