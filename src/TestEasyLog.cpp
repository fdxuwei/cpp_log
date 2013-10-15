#include "EasyLog.h"
#include <iostream>
using namespace std;

using namespace EasyLog;

int main()
{
	time_t tStart = time(NULL);
	Log aLog;
	FileAppender *fa = new FileAppender();
	fa->SetMaxFileLife(2);
	fa->SetCompress(true);
#ifdef WIN32
	fa->SetDir("e:\\file_log");
#else
	fa->SetDir("file_log");
#endif
	
	fa->SetPrefixName("test");
	aLog.AddAppender(AppenderPtr(fa));

	QueuedFileAppender *qfa = new QueuedFileAppender();
	qfa->SetMaxFileLife(2);
	qfa->SetCompress(true);
#ifdef WIN32
	qfa->SetDir("e:\\queue_log");
#else
	qfa->SetDir("queue_log");
#endif
	qfa->SetPrefixName("test");
	aLog.AddAppender(AppenderPtr(qfa));

//	aLog.AddAppender(AppenderPtr(new ConsoleAppender()));
	aLog.SetLogLevel(LOG_LEVEL_ALL);
	for(int i = 0; i < 100; i++)
	{
		LOG_DEBUG(aLog, "This a tst for log debug");
		LOG_INFO(aLog, "This a test for log info");
		LOG_WARN(aLog, "This a test for log warn");
		LOG_ERROR(aLog, "This a test for log error");
		LOG_FATAL(aLog, "This a test for log fatal");
	}

	cout << "cost:" << time(NULL) - tStart << endl;
	return 0;
}