/*************************************************************************
    > File Name: log.cpp
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年10月09日 星期三 15时34分47秒
 ************************************************************************/
#include "log.h"
#include "defines.h"
#include <syslog.h>
#include <string.h>

void InitLog()
{
	zlog_init("/usr/local/apache_conf/zlog.conf");
//	openlog("apache_conf",LOG_PID | LOG_CONS,LOG_USER);
}

zlog_category_t* GetCategory(char* category)
{
	return zlog_get_category(category);
//	return NULL;
}

void UnInitLog()
{
	zlog_fini();
//	closelog();
}


void WriteLog(zlog_category_t *c,int level,char *log)
//void WriteLog(int* c,int level,char *log)
{
	if(c)
	{
		switch(level)
		{
			case DEBUG:
				zlog_debug(c,log);
				break;
			case INFO:
				zlog_info(c,log);
				break;
			case NOTICE:
				zlog_info(c,log);
				break;
			case WARN:
				zlog_info(c,log);
				break;
			case ERROR:
				zlog_info(c,log);
				break;
			case FATAL:
				zlog_fatal(c,log);
				break;
		}
	}
	/*	switch(level)
		{
			case DEBUG:
				syslog(LOG_DEBUG,log,strlen(log));
				break;
			case INFO:
				syslog(LOG_INFO,log,strlen(log));
				break;
			case NOTICE:
				syslog(LOG_NOTICE,log,strlen(log));
				break;
			case WARN:
				syslog(LOG_WARNING,log,strlen(log));
				break;
			case ERROR:
				syslog(LOG_ERR,log,strlen(log));
				break;
			case FATAL:
				syslog(LOG_EMERG,log,strlen(log));
				break;
		}*/
}
