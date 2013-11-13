/*************************************************************************
    > File Name: vhost.h
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年11月07日 星期四 16时43分28秒
 ************************************************************************/
#ifndef _VHOST_H
#define _VHOST_H
#include <vector>
#include <string>
using namespace std;

bool CreateVHost(vector<pair<string,string> > &vt_param,string &error);

bool DeleteVHost(vector<pair<string,string> > &vt_param,string &error);
#endif

