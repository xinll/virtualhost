/*************************************************************************
    > File Name: passwd.h
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年11月07日 星期四 13时18分42秒
 ************************************************************************/
#ifndef _PASSWD_H
#define _PASSWD_H
#include<vector>
#include<string>
using namespace std;

bool ProcPasswd(vector<pair<string,string> > &vt_param,string &error);

#endif
