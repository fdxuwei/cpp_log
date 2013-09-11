#include <ctime>
#include <iomanip>
#include <fstream>
#include <boost/bind.hpp>
#include "EasyLog.h"

using namespace std;

namespace EasyLog
{
	// member functions for Log
	Log::Log()
		:m_LogLevel(LOG_LEVEL_FATAL)
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

	// member functions for FileAppender
	FileAppender::FileAppender(const std::string& sFileName)
		: m_sFileName (sFileName)
		, m_MaxFileSize(10*1024*1024)// default max log file size is 10M
	{
	}

	FileAppender::~FileAppender()
	{
		m_filestream.close();
	}

	void FileAppender::SetMaxFileSize(long size)
	{
		m_MaxFileSize = size;
	}

	void FileAppender::Open()
	{
		m_filestream.open(m_sFileName.c_str(), ios_base::app);
		if(m_filestream.fail())
		{
			m_filestream.clear();
			cout << "open file failed: " << m_sFileName << endl;
		}
	}

	void FileAppender::Close()
	{
		streampos pos =  m_filestream.tellp();
		streamoff off = pos;
		if(pos > m_MaxFileSize)
		{
			m_filestream.close();
			string sAnotherFileName = m_sFileName+"2";
			if(rename(m_sFileName.c_str(), sAnotherFileName.c_str()))
			{
				remove(sAnotherFileName.c_str());
				int result = rename(m_sFileName.c_str(), sAnotherFileName.c_str());
				if(result)
				{
					cout << "rename failed: " << result << endl;
				}
			}
		}
		else
		{
			m_filestream.close();
		}
	}

	void FileAppender::Write(const std::string& msg)
	{
		m_filestream << msg;
	}

	// QueuedAppender
	QueuedFileAppender::QueuedFileAppender(const std::string& sFileName)
		: m_FileAppender (sFileName)
		, m_bRun (true)
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
		m_FileAppender.Open();
		while(m_Queue.PopMsg(sMsg))
		{
			m_FileAppender.Write(sMsg);
		}
		m_FileAppender.Close();
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
#ifdef WIN32
		localtime_s(&tmNow, &ttNow);
#else
		localtime_r(&ttNow, &tmNow);
#endif
		stime << std::setfill('0') << std::setw(4) << tmNow.tm_year+1900 << "/";
		stime << std::setfill('0') << std::setw(2) << tmNow.tm_mon+1 << "/";
		stime << std::setfill('0') << std::setw(2) << tmNow.tm_mday << " ";
		stime << std::setfill('0') << std::setw(2) << tmNow.tm_hour << ":";
		stime << std::setfill('0') << std::setw(2) << tmNow.tm_min << ":";
		stime << std::setfill('0') << std::setw(2) << tmNow.tm_sec;
		return stime.str();
	}
}
