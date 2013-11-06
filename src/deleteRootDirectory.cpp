/*************************************************************************
    > File Name: deleteRootDirectory.cpp
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年11月04日 星期一 11时15分00秒
 ************************************************************************/
#include "deleteRootDirectory.h"
#include "tools.h"

static char deleteDirectory[] = "deleteDirectoryLog";
void DeleteRootDirectory(vector<pair<string,string> >&vt_param,string &errInfo)
{
	WriteParam(deleteDirectory,vt_param,"");
	if(vt_param.size() < 3)
	{
		errInfo.append("too less param.");
		WriteParam(deleteDirectory,vt_param,"failed");
		return;
	}

	string userName = vt_param[2].second;
	string path = GetEnvVar("USER_ROOT");
	if(path.empty())
	{
		path = "/var/www/virtual/";
	}
	path.append(userName);
	path.append("/home/wwwroot");
	RmDir(path.c_str());
	WriteParam(deleteDirectory,vt_param,"success");
}
