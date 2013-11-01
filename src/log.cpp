/*************************************************************************
    > File Name: log.cpp
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年10月09日 星期三 15时34分47秒
 ************************************************************************/
#include "log.h"
#include "defines.h"
#include <string.h>
#include <pthread.h>
#include <vector>
#include <string>
#include <syslog.h>
using namespace std;
static pthread_mutex_t mutex_log;
static vector<pair<string,zlog_category_t*> > vt_category;

void InitLog()
{
	pthread_mutex_init(&mutex_log,NULL);
	zlog_init("/usr/local/apache_conf/zlog.conf");
}

zlog_category_t* GetCategory(const char* category)
{
	zlog_category_t* ret = NULL;
	pthread_mutex_lock(&mutex_log);
	vector<pair<string,zlog_category_t*> >::iterator it = vt_category.begin();
	for(; it != vt_category.end(); it++)
	{
		if(strcmp((*it).first.c_str(),category) == 0)
		{
			ret = (*it).second;
			break;
		}
	}
	if(ret == NULL)
	{
		ret = zlog_get_category(category);
		pair<string,zlog_category_t*> p;
		p.first = category;
		p.second = ret;
		vt_category.push_back(p);
	}
	pthread_mutex_unlock(&mutex_log);
	return ret;
}

void UnInitLog()
{
	zlog_fini();
}


void WriteLog(const char *category,int level,const char *log)
{
	zlog_category_t* c = GetCategory(category);
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
