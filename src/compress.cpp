/*************************************************************************
    > File Name: compress.cpp
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年11月04日 星期一 14时46分06秒
 ************************************************************************/

#include "compress.h"
#include "tools.h"
#include "log.h"
#include <sys/types.h>
#include <dirent.h>
#include <stdlib.h>

static char log[] = "compress";
bool Compress(vector<pair<string,string> > &vt_param, string &errInfo)
{
	WriteParam(log,vt_param,"");
	
	if(!CheckParam(vt_param,5,errInfo))
	{
		return false;
	}
	string userName = GetValue(USERNAME,vt_param);
	string zipName = GetValue(FILENAME,vt_param);
	string dir = GetValue(DIRECTORY,vt_param);
	
	if(!ValidateParamEmpty(userName.c_str()) || !ValidateParamEmpty(zipName.c_str()))
	{
		errInfo.append("ftpName or zipName invalid.");
		WriteLog(log,ERROR,"ftpName or zipName invalid.");
		return false;
	}

	string path = GetEnvVar("USER_ROOT");
	MakePath(path,userName);
	MakePath(path,"home");
	
	string zipPath = path;
	MakePath(zipPath,zipName);

	MakePath(path,dir);
	
	DIR *d;
	if((d = opendir(path.c_str())) == NULL)
	{
		errInfo.append("directory not exist.");
		WriteLog(log,ERROR,"directory not exist.");
		return false;
	}

	closedir(d);
	char buf[256];
	chdir(path.c_str());
	if(snprintf(buf,256,"zip -r -q %s ./* > /dev/null",zipPath.c_str()) >= 256)
	{
		chdir("/");
		errInfo.append("the fileName is too long.");
		WriteLog(log,ERROR,"the fileName is too long.");
		return false;
	}
	int ret = system(buf);
	chdir("/");

	if(ret != -1 && WIFEXITED(ret) && (WEXITSTATUS(ret) == 0 || WEXITSTATUS(ret) == 12))
	{
		snprintf(buf,256,"chown %s:%s %s",userName.c_str(),userName.c_str(),zipPath.c_str());
		ret = system(buf);
		if(ret != -1 && WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
		{
			WriteParam(log,vt_param,"success");
			return true;
		}
		else
		{
			errInfo.append("can't change the owner or group.");
			return false;
		}
	}
	else
		return false;
}

string GetCmd(string &str,string &filePath,string &fileName)
{
	string ret = "";
	if(str.find("Zip") != string::npos)
	{
		ret.append("unzip -u ");
		ret.append(filePath);
		ret.append(" > /dev/null");
	}
	else if(str.find("RAR") != string::npos)
	{
		ret.append("rar x -o+ ");
		ret.append(filePath);
	}
	else if(str.find("gzip") != string::npos)
	{
		vector<string> vt;
		SplitByComas(fileName,vt,'.');
		if(StrInVt("tar",vt))
		{
			ret.append("tar -xzf ");
			ret.append(filePath);
		}
		else
		{
			ret.append("gzip -d ");
			ret.append(filePath);
		}
	}
	else if(str.find("bzip2") != string::npos)
	{
		vector<string> vt;
		SplitByComas(fileName,vt,'.');
		if(StrInVt("tar",vt))
		{
			ret.append("tar -xjf ");
			ret.append(filePath);
		}
		else
		{
			ret.append("bunzip ");
			ret.append(filePath);
		}
	}
	else if(str.find("POSIX") != string::npos)
	{
		ret.append("tar -xf ");
		ret.append(filePath);
	}
	return ret;
}

bool UnCompress(vector<pair<string,string> > &vt_param, string &errInfo)
{
	WriteParam(log,vt_param,"");
	
	if(!CheckParam(vt_param,5,errInfo))
	{
		return false;
	}
	string userName = GetValue(USERNAME,vt_param);
	string zipName = GetValue(FILENAME,vt_param);
	string dir = GetValue(DIRECTORY,vt_param);
	
	if(!ValidateParamEmpty(userName.c_str()) || !ValidateParamEmpty(zipName.c_str()))
	{
		errInfo.append("ftpName or zipName invalid.");
		WriteLog(log,ERROR,"ftpName or zipName invalid.");
		return false;
	}
	
	string path = GetEnvVar("USER_ROOT");
	MakePath(path,userName);
	MakePath(path,"home");
	
	string zipPath = path;
	MakePath(zipPath,zipName);

	MakePath(path,dir);

	//判断文件是否存在
	if(access(zipPath.c_str(),W_OK) != 0)
	{
		errInfo.append("the file does not exist.");
		WriteLog(log,ERROR,"the file does not exist");
		return false;
	}

	time_t t = time(NULL);
	char cmd[256];
	sprintf(cmd,"file %s > /tmp/%d",zipPath.c_str(),t);

	int ret = system(cmd);
	if(ret != -1 && WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
	{
		vector<string> vt;
		char fileName[256];
		sprintf(fileName,"/tmp/%d",t);
		if(!ReadFile(&vt,fileName) || vt.size() != 1)
		{
			errInfo.append("can't get the file's type.");
			WriteLog(log,ERROR,"can't get the file's type");
			return false;
		}
		unlink(fileName);
		string tmp = GetCmd(vt[0],zipPath,zipName);
		if(tmp.empty())
		{
			errInfo.append("unknow compress format.");
			WriteLog(log,ERROR,"unknow compress format.");
			return false;
		}
		DIR *dir = NULL;
		if((dir = opendir(path.c_str())) == NULL)
		{
			sprintf(cmd,"mkdir -p %s",path.c_str());
			ret = system(cmd);
		}
		else
		{
			closedir(dir);
		}
		if(chdir(path.c_str()) != 0)
		{
			return false;
		}
		ret =system(tmp.c_str());
		chdir("/");
		if(ret != -1 && WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		errInfo.append("can't get the file's type.");
		WriteLog(log,ERROR,"can't get the file's type");
		return false;
	}
	
}
