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
	string tmp(firstCharNotSpace,data - firstCharNotSpace);
	result.push_back(tmp);
}

/*void Split(string source,vector<string> &result)
{
	const char* data = source.c_str();
	const char* firstCharNotSpace = data;
	while(*data != '\0')
	{
		if((*data == 32 || *data == 9) && data == firstCharNotSpace) //空格
		{
			data++;
			firstCharNotSpace = data;
			continue;
		}
		else if((*data == 32 || *data == 9) && data != firstCharNotSpace)
		{
			string dest(firstCharNotSpace,data - firstCharNotSpace);
			result.push_back(dest);
			data++;
			firstCharNotSpace = data;
			continue;
		}
		else
		{
			data++;
			if(firstCharNotSpace == '\0')
				firstCharNotSpace = data;
		}
	}
}*/

bool IsEqualString(string first,string second)
{
	return strcasecmp(first.c_str(),second.c_str()) == 0;
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

extern pthread_mutex_t mutex;
extern vector<string> vt_fileName;
bool ConfirmFileUsingState(string &fileName)
{
	pthread_mutex_lock(&mutex);
	vector<string>::iterator it = vt_fileName.begin();
	for(; it != vt_fileName.end(); it++)
	{
		if((*it).compare(fileName) == 0)
		{
			pthread_mutex_unlock(&mutex);
			return true;
		}
	}
	if(!fileName.empty())
		vt_fileName.push_back(fileName);
	pthread_mutex_unlock(&mutex);
	return false;
}

void CancleFileUsingState(string &fileName)
{
	pthread_mutex_lock(&mutex);
	vector<string>::iterator it = vt_fileName.begin();
	for(; it != vt_fileName.end(); it++)
	{
		if((*it).compare(fileName) == 0)
		{
			vt_fileName.erase(it);
			break;
		}
	}
	pthread_mutex_unlock(&mutex);
}

bool UpLoadFile()
{
	/*CFTP ftpClient;
	int err = ftpClient.ftp_connect("172.16.98.128");
	if(err)
	{
		//连接ftp错误
		syslog(LOG_ERR,"can't connect the ftp server!!!");
		return false;
	}
	//err = ftpClient.ftp_login("w11","JHF\\$(\\$FJEJ*4835hg4");
	err = ftpClient.ftp_login("xinll","meicheng");
	if(err)
	{
		//登陆错误
		syslog(LOG_ERR,"can't login the ftp server!!!");
		return false;
	}
	err = ftpClient.ftp_upload("/backup_main/vhost-conf.tar.gz","/backup_main","vhost-conf.tar.gz");
	
	if(err)
	{
		//上传文件错误
		syslog(LOG_ERR,"upload file failed!!!");
		return false;
	}
	ftpClient.ftp_quit();*/
	return true;
}
