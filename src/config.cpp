/*************************************************************************
    > File Name: config.cpp
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年09月18日 星期三 10时07分03秒
 ************************************************************************/

#include <iostream>
#include "config.h"
#include "tools.h"
#include <string.h>
using namespace std;

bool Config::LoadConfigFile(string file)
{
	return ReadFile(&vt_config,file.c_str());
}

string Config::GetValue(string key)
{
	vector<pair<string,string> >::iterator it = vt_configInfo.begin();

	for(;it != vt_configInfo.end();it++)
	{
		if(strcasecmp(key.c_str(),(*it).first.c_str()) == 0)
		{
			return (*it).second;
		}
	}
	
	string retValue = "";
	vector<string>::iterator iter = vt_config.begin();
	for(;iter != vt_config.end(); iter++)
	{
		if((*iter).find(key) == 0)
		{
			size_t found = (*iter).find("=");
			if(found != string::npos)
			{
				retValue = (*iter).substr(found + 1,(*iter).length() - strlen(NEWLINE) - found - 1);
				pair<string,string> p = make_pair(key,retValue);
				vt_configInfo.push_back(p);
			}
			vt_config.erase(iter);
		}
	}
	return retValue;
}

