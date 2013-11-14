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
#include <sys/stat.h>

static char log[] = "compress";
bool Compress(vector<pair<string,string> > &vt_param, string &errInfo)
{
	WriteParam(log,vt_param,"");
	
	string userName = GetValue(USERNAME,vt_param);
	string zipName = GetValue(FILENAME,vt_param);
	string dir = GetValue(DIRECTORY,vt_param);
	
	if(!ValidateParamEmpty(userName.c_str()) || !ValidateParamEmpty(zipName.c_str()))
	{
		errInfo.append("compress failed.");
		WriteParam(log,vt_param,"failed. ftpName or zipName invalid.");
		return false;
	}

	string path = GetEnvVar("USER_ROOT");
	if(path.empty())
		path = USER_ROOT;

	MakePath(path,userName);
	MakePath(path,"home");
	
	string zipPath = path;
	MakePath(zipPath,zipName);

	MakePath(path,dir);
	
	DIR *d;
	if((d = opendir(path.c_str())) == NULL)
	{
		dir.append(" does not exist");
		errInfo.append(dir);

		WriteParam(log,vt_param,"failed. dir does not exist");
		return false;
	}

	closedir(d);
	char buf[256];
	chdir(path.c_str());
	if(snprintf(buf,256,"zip -r -q %s ./* > /dev/null",zipPath.c_str()) >= 256)
	{
		chdir("/");
		char info[] = "the fileName is too long.";
		errInfo.append(info);
		WriteParam(log,vt_param,"failed. the fileName is too long");
		return false;
	}
	int ret = system(buf);
	chdir("/");

	if(ret != -1 && WIFEXITED(ret) && (WEXITSTATUS(ret) == 0 || WEXITSTATUS(ret) == 12))
	{
		snprintf(buf,256,"chown %s:%s %s > /dev/null",userName.c_str(),userName.c_str(),zipPath.c_str());
		ret = system(buf);
		if(ret != -1 && WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
		{
			WriteParam(log,vt_param,"success");
			return true;
		}
		else
		{
			unlink(zipPath.c_str());
			errInfo.append("compress failed.");
			WriteParam(log,vt_param,"failed. chown failed.");
			return false;
		}
	}
	else
	{
		errInfo.append("compress failed.");
		WriteParam(log,vt_param,"failed.");
		return false;
	}
}

string GetCmd(string &str,string &filePath,string &fileName)
{
	string ret = "";
	if(str.find("Zip") != string::npos)
	{
		ret.append("unzip -o ");
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
	
	string userName = GetValue(USERNAME,vt_param);
	string zipName = GetValue(FILENAME,vt_param);
	string dir = GetValue(DIRECTORY,vt_param);
	
	if(!ValidateParamEmpty(userName.c_str()) || !ValidateParamEmpty(zipName.c_str()))
	{
		errInfo.append("compress failed.");
		WriteParam(log,vt_param,"failed. ftpName or zipName invalid.");
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
		char info[] = "the file does not exist.";
		errInfo.append(info);
		WriteParam(log,vt_param,"failed. the file does not exist");
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
			errInfo.append("uncompress failed.");
			WriteParam(log,vt_param,"failed. can't get the file's type");
			unlink(fileName);
			return false;
		}
		unlink(fileName);
		string tmp = GetCmd(vt[0],zipPath,zipName);
		if(tmp.empty())
		{
			char info[] = "unknow compress format.";
			errInfo.append(info);
			WriteParam(log,vt_param,"failed. unknow compress format");
			return false;
		}
		DIR *dir = NULL;
		if((dir = opendir(path.c_str())) == NULL)
		{
			sprintf(cmd,"mkdir -p -m 755 %s",path.c_str());
			ret = system(cmd);
		}
		else
		{
			closedir(dir);
		}

		time_t t = time(NULL);
		char tmpDir[PATH_MAX];
		sprintf(tmpDir,"/tmp/%d",t);
		if(mkdir(tmpDir,S_IWUSR | S_IRUSR | S_IRGRP | S_IWGRP) != 0)
		{
			errInfo.append("uncpmpress failed.");
			WriteParam(log,vt_param,"failed. create the temp directory failed");
			return false;
		}
		if(chdir(tmpDir) != 0)
		{
			errInfo.append("uncpmpress failed.");
			WriteParam(log,vt_param,"failed. change to the temp directory failed");
			return false;
		}
		
		ret =system(tmp.c_str());
		chdir("/");
		if(ret != -1 && WIFEXITED(ret) && (WEXITSTATUS(ret) == 0 || WEXITSTATUS(ret) == 12))
		{
			sprintf(cmd,"chown -R %s:%s %s > /dev/null",userName.c_str(),userName.c_str(),tmpDir);
			
			ret = system(cmd);
			if(ret != -1 && WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
			{
				sprintf(cmd,"mv %s/* %s",tmpDir,path.c_str());
				ret = system(cmd);
				sprintf(cmd,"rm -rf %s",tmpDir);
				system(cmd);
				if(ret != -1 && WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
				{
					WriteParam(log,vt_param,"success");
					return true;
				}
				else
				{
					WriteParam(log,vt_param,"failed. move form temp directory to destination failed.");
					errInfo.append("uncompress failed.");
					return false;
				}
			}
			else
			{
				WriteLog(log,ERROR,cmd);
				WriteParam(log,vt_param,"failed. can't change the owner or group.");
				errInfo.append("uncompress failed.");
				return false;
			}
			return true;
		}
		else
		{
			WriteLog(log,ERROR,tmp.c_str());
			errInfo.append("uncompress failed.");
			WriteParam(log,vt_param,"failed");
			return false;
		}
	}
	else
	{
		errInfo.append("uncompress failed.");
		WriteParam(log,vt_param,"failed. can't get the file's type");
		return false;
	}
}
