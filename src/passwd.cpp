/*************************************************************************
    > File Name: passwd.cpp
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年11月07日 星期四 13时18分34秒
 ************************************************************************/

#include "passwd.h"
#include "tools.h"
#include "log.h"
#include "defines.h"
#include <stdlib.h>

static char log[] = "passwd";
bool ProcPasswd(vector<pair<string,string> > &vt_param,string &error)
{
	WriteParam(log,vt_param,"");
	
	string userName = GetValue(USERNAME,vt_param);
	string pwd = GetValue(PWD,vt_param);

	if(!ValidateParamEmpty(userName.c_str()) || !ValidateParamEmpty(pwd.c_str()))
	{
		char info[] = "ftpName or pwd invalid.";
		WriteLog(log,ERROR,info);
		error.append(info);
		return false;
	}

	char buf[256];
	if(snprintf(buf,256,"echo %s |passwd --stdin %s > /dev/null",pwd.c_str(),userName.c_str()) >= 256)
	{
		char info[] = "the passwd or ftpName is too long.";
		error.append(info);
		WriteLog(log,ERROR,info);
		return false;
	}
	
	int ret = system(buf);
	
	if(ret != -1 && WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
	{
		if(!BakSysInfo())
		{
			char info[] = "bak the sys failed.";
			WriteLog(log,ERROR,info);
			//error.append(info);
			return true;
		}
		else
		{
			WriteParam(log,vt_param,"success");
			return true;
		}
	}
	else
	{
		char info[] = "change the password failed.";
		WriteLog(log,ERROR,info);
		error.append(info);
		return false;
	}
}
