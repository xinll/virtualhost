/*************************************************************************
    > File Name: config.cpp
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年09月18日 星期三 10时07分03秒
 ************************************************************************/

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
/*	vector<pair<string,string> >::iterator it = vt_configInfo.begin();

	for(;it != vt_configInfo.end();it++)
	{
		if(strcasecmp(key.c_str(),(*it).first.c_str()) == 0)
		{
			return (*it).second;
		}
	}
*/	
	string retValue = "";
	vector<string>::iterator iter = vt_config.begin();
	vector<string> vt;
	for(;iter != vt_config.end(); iter++)
	{
		vt.clear();
		SplitByComas((*iter),vt,'=');
		if(vt.size() != 2)
		{
			continue;
		}
		else
		{
			if(vt[0].compare(key) == 0)
			{
				retValue = vt[1].substr(0,vt[1].length() - strlen(NEWLINE));
			}
		}
	}
	return retValue;
}

