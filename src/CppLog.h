#ifndef __CPP_LOG_H__
#define __CPP_LOG_H__

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <queue>
#include <memory>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
 
namespace CppLog
{
	class Appender;
	class ConsoleAppender;
	class FileAppender;
	class QueuedFileAppender;
	class FileManager;

	// data types
	typedef boost::shared_ptr<FileManager> FileManagerPtr; 
	typedef boost::shared_ptr<ConsoleAppender> ConsoleAppenderPtr;
 	typedef boost::shared_ptr<FileAppender> FileAppenderPtr;
 	typedef boost::shared_ptr<QueuedFileAppender> QueuedFileAppenderPtr;
	typedef boost::shared_ptr<Appender> AppenderPtr;
	typedef std::vector<AppenderPtr> AppenderList;
	typedef boost::mutex LogMutex;
	// log level
	enum LOG_LEVEL
	{
		LOG_LEVEL_DEBUG,
		LOG_LEVEL_INFO,
		LOG_LEVEL_WARN,
		LOG_LEVEL_ERROR,
		LOG_LEVEL_FATAL,
		LOG_LEVEL_ALL
	};
	// class Log 
	class Log
	{
	public:
		static Log& Instance()
		{
			static Log aLog;
			return aLog;
		}
		~Log();
		void AddAppender(AppenderPtr appender);
		AppenderList& GetAppenderList();
		void SetLogLevel(LOG_LEVEL level);
		LOG_LEVEL GetLogLevel();
		LogMutex& GetMutex();

	private:
		Log();
		AppenderList m_Appenders;
		LOG_LEVEL m_LogLevel;
		LogMutex m_Mutex;
	};

	// log appender, base class
	class Appender
	{
	public:
		Appender(){}
		virtual ~Appender(){};
		virtual void Write(const std::string& msg) = 0;
//		virtual void Open(){}
//		virtual void Close(){}

	private:
	};
	
	class FileManager
	{
	public:
		FileManager();
		bool SetDir(const std::string& sDir);
		const std::string& GetDir() const { return m_sDir; }
		void SetPrefixName(const std::string& sPrefixName) { m_sPrefixName = sPrefixName; }
		const std::string& GetPrefixName() const { return m_sPrefixName; }
		void SetMaxFileLife(int nDays){ m_nMaxFileLife = nDays; }
		void SetCompress(bool bCompress) { m_bCompress = bCompress; }

		std::string SynthesizeTodyFileName(); // for current date, with path
		void ArrangeFiles(); // clean and compress, if it is set
	private:
		std::string SynthesizeTodyFileStem(); // for current date, without path
		std::string SynthesizeEarlistFileStem();  // for the earlist file, without path
		void ListLogFileStem(std::vector<std::string> &vsLogFiles, std::vector<std::string> &vsZipFiles);
		std::string GetDateString(time_t tt);
		std::string FullPath(const std::string& sName);
		void Compress(const std::string &sStemName);
		void RemoveCompressedFile(const std::string &sStemName);
		std::string m_sDir;
		std::string m_sPrefixName;
		int m_nMaxFileLife; // exist days
		bool m_bCompress;
	};

	// file appender
	class FileAppender : public Appender, public FileManager
	{
	public:
		static FileAppenderPtr Create();
		~FileAppender();
		virtual void Write(const std::string& msg);
	protected:
		FileAppender();
		void Open();
		void Close();
		void WriteWithoutFlush(const std::string& msg);
	private:
		std::ofstream m_filestream;
	};
	// console appender
	class ConsoleAppender : public Appender
	{
	public:
		static ConsoleAppenderPtr Create();
		virtual void Write(const std::string& msg);
	protected:
		ConsoleAppender();
	};

	// queue, thread safe
	class SafeQueue
	{
	public:
		bool PopMsg(std::string& sMsg);
		void PushMsg(const std::string& sMsg);

	private:
		std::stringstream m_ssCache;
		std::deque<std::string> m_MsgQueue;
		LogMutex m_QueueMutex;
	};

	// queued appender, faster than file appender
	class QueuedFileAppender : public FileAppender
	{
	public:
		static QueuedFileAppenderPtr Create();
		~QueuedFileAppender();
		virtual void Write(const std::string& msg);
	protected:
		QueuedFileAppender();
	private:
		SafeQueue m_Queue;
		bool m_bRun;
		boost::shared_ptr<boost::thread> m_ThreadPtr;

		void Sync();
		void Loop();
	};

	// utils
	std::string GetLogTime();

	//
	const std::string c_LogLevelTag[] = {"DEBUG","INFO","WARN","ERROR","FATAL"};

}// end namespace Log

#define LOG_CMD(log,event,level) \
	{\
		boost::lock_guard<CppLog::LogMutex> lock(log.GetMutex());\
		if(log.GetLogLevel() >= level)\
			for(CppLog::AppenderList::iterator it = log.GetAppenderList().begin(); it != log.GetAppenderList().end(); ++it)\
			{\
				std::stringstream ssTemp;\
				ssTemp << CppLog::GetLogTime() << " - " << CppLog::c_LogLevelTag[level] << " - " << event <<\
					" [ " <<  __FILE__ << " : " << __LINE__ << " ]" << "\n";\
				(*it)->Write(ssTemp.str());\
			}\
	}

// log macros, it is recommended that you use these macors to write a log message in your code instead of the member functions 
// event is a stream expression which uses the "<<" operator to link all type of variables £¬for example: LOG_FATAL(log, "Welcome to log," << date << "\n")
#define LOG_FATAL(event) LOG_CMD(CppLog::Log::Instance(),event,CppLog::LOG_LEVEL_FATAL) 
#define LOG_ERROR(event) LOG_CMD(CppLog::Log::Instance(),event,CppLog::LOG_LEVEL_ERROR)
#define LOG_WARN(event) LOG_CMD(CppLog::Log::Instance(),event,CppLog::LOG_LEVEL_WARN)
#define LOG_INFO(event) LOG_CMD(CppLog::Log::Instance(),event,CppLog::LOG_LEVEL_INFO)
#define LOG_DEBUG(event) LOG_CMD(CppLog::Log::Instance(),event,CppLog::LOG_LEVEL_DEBUG)

#endif