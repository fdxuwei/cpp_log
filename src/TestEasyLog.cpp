#include "EasyLog.h"
#include <iostream>
using namespace std;

using namespace EasyLog;

int main()
{
	time_t tStart = time(NULL);
	
	FileAppender *fa = new FileAppender();
	fa->SetMaxFileLife(2);
	fa->SetCompress(true);
#ifdef WIN32
	fa->SetDir("e:\\file_log");
#else
	fa->SetDir("file_log");
#endif
	
	fa->SetPrefixName("test");
	Log::Instance().AddAppender(AppenderPtr(fa));

	QueuedFileAppender *qfa = new QueuedFileAppender();
	qfa->SetMaxFileLife(2);
	qfa->SetCompress(true);
#ifdef WIN32
	qfa->SetDir("e:\\queue_log");
#else
	qfa->SetDir("queue_log");
#endif
	qfa->SetPrefixName("test");
	Log::Instance().AddAppender(AppenderPtr(qfa));

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