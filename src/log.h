/*************************************************************************
    > File Name: log.h
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年10月09日 星期三 15时33分24秒
 ************************************************************************/
#ifndef _LOG_H
#define _LOG_H
#include "zlog.h"

void InitLog(const char* cfgFile);

void UnInitLog();

zlog_category_t* GetCategory(const char* category);

//void WriteLog(zlog_category_t *c,int level,char *log);

void WriteLog(const char *category,int level,const char *log);
#endif
