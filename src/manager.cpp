/*************************************************************************
    > File Name: manager.cpp
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年11月14日 星期四 10时23分26秒
 ************************************************************************/
#include "log.h"
#include "tools.h"
#include "common.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>

static char log[] = "manager";

bool ManageHost(string &userName,string &replace)
{
	string filePath = MakeConfPath(userName);
	if(access(filePath.c_str(),R_OK | W_OK) != 0)
	{
		filePath.append(" can't read or write.");
		WriteLog(log,ERROR,filePath.c_str());
		return false;
	}

	CVirtualHost *virtualHost = NULL;
	if(!InitEnv(&virtualHost,userName,log))
	{
		userName.append(" get virtualHost failed");
		WriteLog(log,ERROR,userName.c_str());
		return false;
	}
	
	string directive = DOCUMENTROOT;

	vector<string>::iterator it = virtualHost->FindGlobalDirective(directive,NULL,0,virtualHost->GetIterator());
	if(it != virtualHost->GetEndIterator())
	{
		virtualHost->EraseItem(it);
	}
	else
	{
		it = virtualHost->GetIterator(2);
	}
	vector<string> tmp;
	tmp.push_back(replace);
	virtualHost->AddDirective(directive,it,tmp,4);

	if(!virtualHost->SaveFile())
	{
		userName.append(" write config file failed");
		WriteLog(log,ERROR,userName.c_str());
		CVirtualHost::ReleaseVirtualHost(userName);
		return false;
	}
	else
	{
		CVirtualHost::ReleaseVirtualHost(userName);
		return true;
	}
}

bool Expired(vector<pair<string,string> > &vt_param,string &error)
{
	string userName = GetValue(USERNAME,vt_param);
	if(!ValidateParamEmpty(userName.c_str()))
	{
		char info[] = "the ftpName can't be empty.";
		error.append(info);
		WriteParam(log,vt_param,"failed. the param is invalid");
		return false;
	}
	
	string expired = GetEnvVar("EXPTME");
	if(expired.empty())
	{
		expired = EXPTME;
	}

	if(!ManageHost(userName,expired))
	{
		error.append("failed to process expired.");
		WriteParam(log,vt_param,"failed");
		return false;
	}
	WriteParam(log,vt_param,"success");
	return true;
}

bool Stop(vector<pair<string,string> > &vt_param,string &error)
{
	string userName = GetValue(USERNAME,vt_param);
	if(!ValidateParamEmpty(userName.c_str()))
	{
		char info[] = "the ftpName can't be empty.";
		error.append(info);
		WriteParam(log,vt_param,"failed. the param is invalid");
		return false;
	}
	
	string expired = GetEnvVar("STOPDIR");
	if(expired.empty())
	{
		expired = STOPDIR;
	}

	if(!ManageHost(userName,expired))
	{
		error.append("failed to process stop.");
		WriteParam(log,vt_param,"failed");
		return false;
	}
	WriteParam(log,vt_param,"success");
	return true;
}

bool Stop2(vector<pair<string,string> > &vt_param,string &error)
{
	string userName = GetValue(USERNAME,vt_param);
	if(!ValidateParamEmpty(userName.c_str()))
	{
		char info[] = "the ftpName can't be empty.";
		error.append(info);
		WriteParam(log,vt_param,"failed. the param is invalid");
		return false;
	}
	
	string expired = GetEnvVar("STOPDIR2");
	if(expired.empty())
	{
		expired = STOPDIR2;
	}

	if(!ManageHost(userName,expired))
	{
		error.append("failed to process stop2.");
		WriteParam(log,vt_param,"failed");
		return false;
	}
	WriteParam(log,vt_param,"success");
	return true;
}

bool Stop3(vector<pair<string,string> > &vt_param,string &error)
{
	string userName = GetValue(USERNAME,vt_param);
	if(!ValidateParamEmpty(userName.c_str()))
	{
		char info[] = "the ftpName can't be empty.";
		error.append(info);
		WriteParam(log,vt_param,"failed. the param is invalid");
		return false;
	}
	
	string expired = GetEnvVar("STOPDIR3");
	if(expired.empty())
	{
		expired = STOPDIR3;
	}

	if(!ManageHost(userName,expired))
	{
		error.append("failed to process stop3.");
		WriteParam(log,vt_param,"failed");
		return false;
	}
	WriteParam(log,vt_param,"success");
	return true;
}

bool Open(vector<pair<string,string> > &vt_param,string &error)
{
	string userName = GetValue(USERNAME,vt_param);
	if(!ValidateParamEmpty(userName.c_str()))
	{
		char info[] = "the ftpName can't be empty.";
		error.append(info);
		WriteParam(log,vt_param,"failed. the param is invalid");
		return false;
	}

	string path = MakeUserRoot(userName);
	if(!ManageHost(userName,path))
	{
		error.append("failed to process open.");
		WriteParam(log,vt_param,"failed");
		return false;
	}
	WriteParam(log,vt_param,"success");
	return true;
}

bool Quota(vector<pair<string,string> > &vt_param,string &error)
{
	string max_trans = GetValue(SIZE,vt_param);
	string userName = GetValue(USERNAME,vt_param);
	
	long long quota = strtoll(max_trans.c_str(),NULL,10) * 1024;
	long long hard_quota = quota + 10240;

	GetMaxTrans(max_trans);

	CVirtualHost *virtualHost = NULL;
	if(!InitEnv(&virtualHost,userName,log))
	{
		userName.append(" get virtualHost failed");
		WriteLog(log,ERROR,userName.c_str());
		return false;
	}
	string directive = "CBandLimit";
	vector<string>::iterator it = virtualHost->FindGlobalDirective(directive,NULL,0,virtualHost->GetIterator());
	if(it != virtualHost->GetEndIterator())
	{
		virtualHost->EraseItem(it);
	}
	else
	{
		it = virtualHost->GetEndIterator();
		it--;
	}
	vector<string> tmp;
	max_trans.append("Mi");
	tmp.push_back(max_trans);
	virtualHost->AddDirective(directive,it,tmp,4);
	
	char cmd[256];
	string filePath = MakeConfPath(userName);
	if(snprintf(cmd,256,"cp %s /tmp/%s",filePath.c_str(),userName.c_str()) >= 256)
	{
		char info[] = "set the quota failed.";
		error.append(info);
		WriteParam(log,vt_param,"failed. the ftpName is too long");
		return false;
	}
	else
	{
		int t = system(cmd);
		if(t != -1 && WIFEXITED(t) && WEXITSTATUS(t) == 0)
		{
			;
		}
		else
		{
			char info[] = "set the quota failed.";
			error.append(info);
			WriteParam(log,vt_param,"failed. bak the config file failed");
			return false;
		}
	}
	if(!virtualHost->SaveFile())
	{
		char info[] = "set the quota failed.";
		error.append(info);
		WriteParam(log,vt_param,"failed. write the config failed");
		CVirtualHost::ReleaseVirtualHost(userName);
		return false;
	}

	CVirtualHost::ReleaseVirtualHost(userName);
	string user_root = GetEnvVar("USER_ROOT");
	bool success = true;
	if(user_root.empty())
	{
		user_root = USER_ROOT;
	}
	if(snprintf(cmd,256,"/usr/sbin/setquota -u %s %lld %lld 40000 45000 %s",userName.c_str(),quota,hard_quota,user_root.c_str()) >= 256)
	{
		char info[] = "set the quota failed.";
		error.append(info);
		WriteLog(log,ERROR,info);
		WriteParam(log,vt_param,"failed. the ftpName is too long");
		success = false;
	}
	if(success)
	{
		int ret = system(cmd);
		if(ret != -1 && WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
		{
			WriteParam(log,vt_param,"success");
		}
		else
		{
			WriteLog(log,ERROR,cmd);
			char info[] = "set the quota failed.";
			error.append(info);
			WriteParam(log,vt_param,"failed");
			success =false;
		}
	}
	if(!success)
	{
		snprintf(cmd,256,"mv /tmp/%s %s",userName.c_str(),filePath.c_str());
		system(cmd);
	}
	return success;
}

bool StartFtp(vector<pair<string,string> > &vt_param,string &error)
{
	string userName = GetValue(USERNAME,vt_param);
	if(!ValidateParamEmpty(userName.c_str()))
	{
		char info[] = "the ftpName can't be empty.";
		error.append(info);
		WriteParam(log,vt_param,"failed. the param is invalid");
		return false;
	}

	string ftpusers = GetEnvVar("FTPUSERS");
	if(ftpusers.empty())
	{
		ftpusers = FTPUSERS;
	}

	char cmd[PATH_MAX];
	if(snprintf(cmd,PATH_MAX,"sed -i \"/^%s$/d\" %s",userName.c_str(),ftpusers.c_str()) >= PATH_MAX)
	{
		error.append("failed to start ftp.");
		WriteParam(log,vt_param,"failed. the ftpName is too long");
		return false;
	}
	int ret = system(cmd);

	if(ret != -1 && WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
	{
		WriteParam(log,vt_param,"success");
		return true;
	}
	else
	{
		error.append("failed to start ftp.");
		WriteParam(log,vt_param,"failed");
		return false;
	}
}

bool StopFtp(vector<pair<string,string> > &vt_param,string &error)
{
	string userName = GetValue(USERNAME,vt_param);
	if(!ValidateParamEmpty(userName.c_str()))
	{
		char info[] = "the ftpName can't be empty.";
		error.append(info);
		WriteParam(log,vt_param,"failed. the param is invalid");
		return false;
	}

	string ftpusers = GetEnvVar("FTPUSERS");
	if(ftpusers.empty())
	{
		ftpusers = FTPUSERS;
	}

	char cmd[PATH_MAX];
	if(snprintf(cmd,PATH_MAX,"echo %s >> %s",userName.c_str(),ftpusers.c_str()) >= PATH_MAX)
	{
		error.append("failed to stop ftp.");
		WriteParam(log,vt_param,"failed. the ftpName is too long");
		return false;
	}
	int ret = system(cmd);

	if(ret != -1 && WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
	{
		WriteParam(log,vt_param,"success");
		return true;
	}
	else
	{
		error.append("failed to stop ftp.");
		WriteParam(log,vt_param,"failed");
		return false;
	}
}

bool Manager(vector<pair<string,string> > &vt_param,string &error,bool &restartFTP)
{
	WriteParam(log,vt_param,"");

	string action = GetValue(ACTION,vt_param);
	if(!ValidateParamEmpty(action.c_str()))
	{
		char info[] = "the operation can't be empty.";
		error.append(info);
		WriteParam(log,vt_param,"failed. the param is invalid");
		return false;
	}
	
	restartFTP = false;
	if(action.compare(EXPIRED) == 0)
	{
		return Expired(vt_param,error);
	}
	else if(action.compare(STOP) == 0)
	{
		return Stop(vt_param,error);
	}
	else if(action.compare(STOP2) == 0)
	{
		return Stop2(vt_param,error);
	}
	else if(action.compare(STOP3) == 0)
	{
		return Stop3(vt_param,error);
	}
	else if(action.compare(OPEN) == 0)
	{
		return Open(vt_param,error);
	}
	else if(action.compare(QUOTA) == 0)
	{
		return Quota(vt_param,error);
	}
	else if(action.compare(STARTFTP) == 0)
	{
		restartFTP = true;
		return StartFtp(vt_param,error);
	}
	else if(action.compare(STOPFTP) == 0)
	{
		restartFTP = true;
		return StopFtp(vt_param,error);
	}
	else
	{
		error.append("unknow manager function.");
		WriteParam(log,vt_param,"failed");
		return false;
	}
 }
