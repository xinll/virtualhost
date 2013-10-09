/*************************************************************************
    > File Name: log.cpp
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年10月09日 星期三 15时34分47秒
 ************************************************************************/
#include "zlog.h"
#include "defines.h"

zlog_category_t *c;

void InitLog()
{
	zlog_init("/usr/local/apache_conf/zlog.conf");
}

zlog_category_t* GetCategory(char* category)
{
	return zlog_get_category(category);
}

void UnInitLog()
{
	zlog_fini();
}


void WriteLog(zlog_category_t* c,int level,char *log)
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
}
