#include "EasyLog.h"
#include <iostream>
using namespace std;

using namespace EasyLog;

int main()
{
	time_t tStart = time(NULL);
	Log aLog;
	aLog.AddAppender(AppenderPtr(new FileAppender("test.log")));
	aLog.AddAppender(AppenderPtr(new QueuedFileAppender("test_queue.log")));
	aLog.AddAppender(AppenderPtr(new ConsoleAppender()));
	aLog.SetLogLevel(LOG_LEVEL_ALL);
	for(int i = 0; i < 100 ; i++)
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