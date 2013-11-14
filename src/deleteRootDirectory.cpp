/*************************************************************************
    > File Name: deleteRootDirectory.cpp
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年11月04日 星期一 11时15分00秒
 ************************************************************************/
#include "deleteRootDirectory.h"
#include "tools.h"

static char log[] = "deleteDirectoryLog";
void DeleteRootDirectory(vector<pair<string,string> >&vt_param,string &errInfo)
{
	WriteParam(log,vt_param,"");

	string userName = GetValue("USERNAME",vt_param);
	if(!ValidateParamEmpty(userName.c_str()))
	{
		WriteParam(log,vt_param,"failed. the ftpName can't be empty");
		errInfo.append("delete the directory failed.");
		return;
	}
	string path = GetEnvVar("USER_ROOT");
	if(path.empty())
	{
		path = "/var/www/virtual/";
	}
	path.append(userName);
	path.append("/home/wwwroot");
	RmDir(path.c_str());
	WriteParam(log,vt_param,"success");
}
