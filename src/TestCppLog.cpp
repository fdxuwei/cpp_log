#include "CppLog.h"
#include <iostream>
using namespace std;

using namespace CppLog;

int main()
{
	time_t tStart = time(NULL);
	
	// create file appender£¬so that log messages are written to a file instantaneity
	FileAppenderPtr fa = FileAppender::Create();
	fa->SetMaxFileLife(2);
	fa->SetCompress(true);
#ifdef WIN32
	fa->SetDir("e:\\file_log");
#else
	fa->SetDir("file_log");
#endif
	
	fa->SetPrefixName("test");
	Log::Instance().AddAppender(fa);

	// create queued file appender£¬so that log messages are first written to a buffer, and are flushed to disk file periodically,
	// QueuedFileAppenderPtr is quick than FileAppender ,but not real time
	QueuedFileAppenderPtr qfa = QueuedFileAppender::Create();
	qfa->SetMaxFileLife(2);
	qfa->SetCompress(true);
#ifdef WIN32
	qfa->SetDir("e:\\queue_log");
#else
	qfa->SetDir("queue_log");
#endif
	qfa->SetPrefixName("test");
	Log::Instance().AddAppender(qfa);

//	aLog.AddAppender(AppenderPtr(new ConsoleAppender()));
	Log::Instance().SetLogLevel(LOG_LEVEL_ALL);
	for(int i = 0; i < 100; i++)
	{
		LOG_DEBUG("This a tst for log debug");
		LOG_INFO("This a test for log info");
		LOG_WARN("This a test for log warn");
		LOG_ERROR("This a test for log error");
		LOG_FATAL("This a test for log fatal");
	}

	cout << "cost:" << time(NULL) - tStart << endl;
	return 0;
}