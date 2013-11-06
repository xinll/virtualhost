/*************************************************************************
    > File Name: ../src/config.h
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年09月18日 星期三 10时00分56秒
 ************************************************************************/
#ifndef _CONFIG_H
#define _CONFIG_H
#include <vector>
#include <string>
using namespace std;

class Config
{
public:
	bool LoadConfigFile(string file = "/usr/local/apache_conf/cfg/apache_conf.conf");
	string GetValue(string key);
private:
	vector<pair<string,string> > vt_configInfo;
	vector<string>               vt_config;
};
#endif
