/*************************************************************************
    > File Name: tools.cpp
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年09月16日 星期一 09时53分12秒
 ************************************************************************/

#include "tools.h"
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <sys/stat.h>
#include <dirent.h>
#include "ftp.h"
bool ReadFile(vector<string> *vt_conf,const char *fileName)
{

	fstream f;
	f.open(fileName,ios::in | ios::out);
	if(!f)	
	{
		//打开文件出错,......
		return false;
	}
	
	while(!f.eof())
	{
		string line;
		getline(f,line);
		if(line == NEWLINE)
			continue;
		line.append(NEWLINE);
		vt_conf->push_back(line);
	}
	f.close();
	return true;
}


bool  WriteFile(vector<string> *vt_conf,const char *fileName)
{
	fstream fs;
	fs.open(fileName,ios::out | ios::trunc);
	if(!fs)
		return false;
	vector<string>::iterator it;
	for(it = vt_conf->begin(); it != vt_conf->end(); it++)
	{
		if((*it) != NEWLINE)
			fs << (*it);
	}
	fs.close();
	return true;
}

bool ProcParam(char *param,vector< pair<string,string> > &vt_param)
{
	char *tmp = param;
	char *p = NULL;
	while(p = strstr(tmp,SPLIT))
	{
		if(p == tmp)
		{
			tmp += strlen(SPLIT);
			continue;
		}
		string str(tmp,p - tmp);

		size_t found = str.find("=");
		if(found == string::npos)
		{
			return false;
		}
		else
		{
			string key = str.substr(0,found);
			string value = str.substr(found + 1);
			pair<string,string> m = make_pair(key,value);
			vt_param.push_back(m);
		}
		tmp = p + strlen(SPLIT);
		//add to .\r\n
		if(strstr(tmp,PARAMEND) == tmp)
			break;
	}
	return true;
}

extern string dirPath;
string MakeConfPath(string &ftpName)
{
	string path = dirPath + ftpName + ".conf";
	return path;

}
//以"开头的字符串中遇到空格或tab不分割
void Split(string source,vector<string> &result)
{
	const char* data = source.c_str();
	const char* firstCharNotSpace = data;
	bool  marks = false;
	bool  preIsSpace = true;
	while(*data != '\0')
	{
		if(*data == 34) //如果是引号
		{
			if(preIsSpace)
			{
				marks = true;
				firstCharNotSpace = data;
				preIsSpace = false;
			}
			data++;
			
			if(marks && (*data == 32 || * data == 9))
			{
				string tmp(firstCharNotSpace,data - firstCharNotSpace);
				result.push_back(tmp);
				marks = false;
				firstCharNotSpace = data;
			}
			continue;
		}
		if(marks)
		{
			data++;
			continue;
		}
		if((*data == 32 || *data == 9) && data == firstCharNotSpace) //空格
		{
			data++;
			firstCharNotSpace = data;
			preIsSpace = true;
			continue;
		}
		else if((*data == 32 || *data == 9) && data != firstCharNotSpace)
		{
			string dest(firstCharNotSpace,data - firstCharNotSpace);
			result.push_back(dest);
			data++;
			firstCharNotSpace = data;
			preIsSpace = true;
			continue;
		}
		else
		{
			preIsSpace = false;
			data++;
		}
	}
	string tmp(firstCharNotSpace,data - firstCharNotSpace); //换行符
	result.push_back(tmp);
}

bool IsEqualString(string first,string second)
{
	const char *f = first.c_str();
	const char *s = second.c_str();
	if(*f == '\"')
	{
		f = first.c_str() + 1;
	}
	if(*s == '\"')
	{
		s = second.c_str() + 1;
	}
	int n = strlen(f) > strlen(s) ? strlen(s) : strlen(f);
	return strncasecmp(first.c_str(),second.c_str(),n) == 0;
}


bool BakConf(string &userName)
{
	chdir("/etc/httpd/vhost.d/");
	string backupWhat = userName;
	backupWhat.append(".conf");
	string backupDir = "/backup_main/vhost-conf.zip";
	string cmd;
	cmd = "zip -qu ";
	cmd.append(backupDir);
	cmd.append(" ");
	cmd.append(backupWhat);
	syslog(LOG_INFO,cmd.c_str());
	int ret = system(cmd.c_str());
	chdir("/");
	if(ret != -1 && WIFEXITED(ret) && (WEXITSTATUS(ret) == 0 || WEXITSTATUS(ret) == 12))
		return true;
	else
		return false;
}

bool RestoreConf(string &userName)
{
	string restoreWhat = userName;
	restoreWhat.append(".conf");
	
	string backupDir = "/backup_main/vhost-conf.zip";
	string cmd = "unzip -qo ";
	cmd.append(backupDir);
	cmd.append(" ");
	cmd.append(restoreWhat);
	cmd.append(" ");
	cmd.append("-d /etc/httpd/vhost.d");
	syslog(LOG_INFO,cmd.c_str());
	int ret = system(cmd.c_str());

	if(ret != -1 && WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
	{
		return true;
	}
	else
		return false;
}

bool StrInVt(string &str,vector<string> &vt)
{
	vector<string>::iterator it = vt.begin();
	for(; it != vt.end(); it++)
	{
		if(IsEqualString((*it),str))
			return true;
	}
	return false;
}

bool UpLoadFile(const char* ftpServer,const char* ftpUser,const char* ftpPwd,const char* file,const char* dir)
{
	CFTP ftpClient;
	int err = ftpClient.ftp_connect(ftpServer);
	if(err)
	{
		//连接ftp错误
		syslog(LOG_ERR,"can't connect the ftp server!!!");
		return false;
	}
	err = ftpClient.ftp_login(ftpUser,ftpPwd);
	if(err)
	{
		//登陆错误
		syslog(LOG_ERR,"can't login the ftp server!!!");
		return false;
	}
	err = ftpClient.ftp_upload(file,dir,file);
	
	if(err)
	{
		//上传文件错误
		syslog(LOG_ERR,"upload file failed!!!");
		return false;
	}
	ftpClient.ftp_quit();
	return true;
}

bool IsDir(const char *path)
{
	struct stat statbuf;
	if(lstat(path,&statbuf) == 0)
	{
		return S_ISDIR(statbuf.st_mode) != 0;
	}
	return false;
}

bool IsFile(const char *path)
{
	struct stat statbuf;
	if(lstat(path,&statbuf) == 0)
	{
		return S_ISREG(statbuf.st_mode) != 0;
	}
	return false;
}

bool IsLnk(const char* path)
{
	struct stat statbuf;
	if(lstat(path,&statbuf) == 0)
	{
		return S_ISLNK(statbuf.st_mode) != 0;
	}
	return false;
}
bool IsSpecial(const char *path)
{
	return strcmp(path,".") == 0 || strcmp(path,"..") == 0;
}

void GetFilePath(const char* path,const char* fileName,char *filePath)
{
	strcpy(filePath,path);
	if(filePath[strlen(path) - 1] != '/')
		strcat(filePath,"/");
	strcat(filePath,fileName);
}

void RmDir(const char *path)
{
	if(IsFile(path) || IsLnk(path))
	{
		remove(path);
		return;
	}
	char filePath[PATH_MAX];
	if(IsDir(path))
	{
		DIR *dir;
		struct dirent *ptr;
		dir = opendir(path);
		while(ptr = readdir(dir))
		{
			if(IsSpecial(ptr->d_name))
				continue;
			GetFilePath(path,ptr->d_name,filePath);
			if(IsDir(filePath))
			{
				RmDir(filePath);
				rmdir(filePath);
			}
			else if(IsFile(filePath) || IsLnk(filePath))
			{
				remove(filePath);
			}
		}
		closedir(dir);
	}
}
