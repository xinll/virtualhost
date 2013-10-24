/*************************************************************************
    > File Name: log.h
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年10月09日 星期三 15时33分24秒
 ************************************************************************/
#include "zlog.h"

void InitLog();

void UnInitLog();

zlog_category_t* GetCategory(char* category);

//void WriteLog(zlog_category_t *c,int level,char *log);

void WriteLog(char *category,int level,char *log);
