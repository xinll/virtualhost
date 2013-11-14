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
#include <sys/stat.h>
#include <dirent.h>
#include "ftp.h"
#include "config.h"
#include "log.h"

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
		if(line == NEWLINE || line.empty())
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
	fs.open(fileName,ios::out);
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
string GetEnvVar(string key)
{
	string value;
	Config config;
	config.LoadConfigFile();
	return  config.GetValue(key);
}

string AddSlash(string &str)
{
	if(str.c_str()[str.size() - 1] != '/')
	{
		str.append("/");
	}
	return str;
}

string MakeConfPath(string ftpName)
{
	string dirPath = GetEnvVar("CONF_DIR");
	if(dirPath.empty())
		dirPath = CONF_DIR;
	AddSlash(dirPath);
	string path = dirPath + ftpName + ".conf";
	return path;

}

//以"开头的字符串中遇到空格或tab不分割
void Split(string source,vector<string> &result)
{
	if(source.rfind(NEWLINE) == source.size() - strlen(NEWLINE))
		source = source.substr(0,source.size() - strlen(NEWLINE));
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
				string t;
				if(*(data-1) == '>')
				{
					if(*(data - 2) == '/')
					{
						string tmp(firstCharNotSpace,data - firstCharNotSpace - 2);
						t = tmp;
					}
					else
					{
						string tmp(firstCharNotSpace,data - firstCharNotSpace - 1);
						t = tmp;
					}
				}
				else
				{
					string tmp(firstCharNotSpace,data - firstCharNotSpace);
					t = tmp;
				}
				result.push_back(t);
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
			string t;
			if(*(data-1) == '>')
			{
				if(*(data - 2) == '/')
				{
					string tmp(firstCharNotSpace,data - firstCharNotSpace - 2);
					t = tmp;
				}
				else
				{
					string tmp(firstCharNotSpace,data - firstCharNotSpace - 1);
					t = tmp;
				}
			}
			else
			{
				string tmp(firstCharNotSpace,data - firstCharNotSpace);
				t = tmp;
			}
		//	string dest(firstCharNotSpace,data - firstCharNotSpace);
			result.push_back(t);
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
	if(firstCharNotSpace + 1 != data)
	{	
		string t;
		if(*(data-1) == '>')
		{
			if(*(data - 2) == '/')
			{
				string tmp(firstCharNotSpace,data - firstCharNotSpace - 2);
				t = tmp;
			}
			else
			{
				string tmp(firstCharNotSpace,data - firstCharNotSpace - 1);
				t = tmp;
			}
		}
		else
		{
			string tmp(firstCharNotSpace,data - firstCharNotSpace);
			t = tmp;
		}
	//	string tmp(firstCharNotSpace,data - firstCharNotSpace); //换行符
		result.push_back(t);
	}
}

bool IsEqualString(string first,string second)
{
/*	if(strict)
	{
		return strcmp(first.c_str(),second.c_str()) == 0;
	}*/
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
	int m = strlen(f);
	int k = strlen(s);
	if(m != k && abs(m-k) != 1 && abs(m-k) != 2)
	{
		return false;
	}
	/*if(m != k)
	{
		if(*(f + m -1) != '\"' || *(f + m - 1) != '>' || *(s + k - 1) != '\"' || *(s + k - 1) != '>')
			return false;
	}*/
	return strncasecmp(f,s,n) == 0;
}

bool BakSysInfo()
{
	string backupDir = GetEnvVar("BACKUPDIR");
	if(backupDir.empty())
		backupDir = BACKUPDIR;

	DIR *dir;
	if(NULL == (dir = opendir(backupDir.c_str())))
	{
		if(mkdir(backupDir.c_str(),0775) != 0)
			return false;
	}
	closedir(dir);

	backupDir = AddSlash(backupDir);
	backupDir.append("sysuser-backup.zip > /dev/null");
	
	chdir("/etc/");
	string cmd = "zip -qu ";
	cmd.append(backupDir);
	cmd.append(" ");
	cmd.append("passwd shadow group gshadow");
	int ret = system(cmd.c_str());
	chdir("/");

	if(ret != -1 && WIFEXITED(ret) && (WEXITSTATUS(ret) == 0 || WEXITSTATUS(ret) == 12))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool BakConf()
{
	string dirPath = GetEnvVar("CONF_DIR");
	if(dirPath.empty())
	{
		dirPath = CONF_DIR;
	}
	if(chdir(dirPath.c_str()) != 0)
		return false;

	string backupDir = GetEnvVar("BACKUPDIR");
	if(backupDir.empty())
		backupDir = BACKUPDIR;
	
	DIR *dir;
	if(NULL == (dir = opendir(backupDir.c_str())))
	{
		if(mkdir(backupDir.c_str(),0775) != 0)
		{
			chdir("/");
			return false;
		}
	}
	else
		closedir(dir);	

	backupDir = AddSlash(backupDir);
	backupDir.append("vhost-conf.zip > /dev/null");

	string cmd;
	cmd = "zip -qu ";
	cmd.append(backupDir);
	cmd.append(" ");
	//改为备份所有
	string backupWhat = "./*";
	cmd.append(backupWhat);
	int ret = system(cmd.c_str());
	chdir("/");
	if(ret != -1 && WIFEXITED(ret) && (WEXITSTATUS(ret) == 0 || WEXITSTATUS(ret) == 12))
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool RestoreConf(string &userName)
{
	string dirPath = GetEnvVar("CONF_DIR");
	if(dirPath.empty())
	{
		dirPath = CONF_DIR;
	}

	string backupDir = GetEnvVar("BACKUPDIR");
	if(backupDir.empty())
		backupDir = BACKUPDIR; 

	string restoreWhat = userName;
	restoreWhat.append(".conf");

	AddSlash(backupDir);
	AddSlash(dirPath);

	backupDir.append("vhost-conf.zip");
	string cmd = "unzip -qo ";
	cmd.append(backupDir);
	cmd.append(" ");
	cmd.append(restoreWhat);
	cmd.append(" ");
	cmd.append("-d ");
	cmd.append(dirPath);
	cmd.append(" > /dev/null");
	int ret = system(cmd.c_str());

	if(ret != -1 && WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
	{
		return true;
	}
	else
		return false;
}

bool StrInVt(string str,vector<string> &vt)
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
		return false;
	}
	err = ftpClient.ftp_login(ftpUser,ftpPwd);
	if(err)
	{
		//登陆错误
		return false;
	}
	err = ftpClient.ftp_upload(file,dir,file);
	
	if(err)
	{
		//上传文件错误
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

void WriteParam(char *category,vector<pair<string,string> > &vt_param,string success)
{
	string param = "process the param:";
	for(int i = 0; i < vt_param.size(); i++)
	{
		param.append(vt_param[i].first);
		param.append("=");
		param.append(vt_param[i].second);
		param.append("|");
	}
	param.append(" ");
	param.append(success);
	char params[4096];
	sprintf(params,"%s",param.c_str());
	WriteLog(category,INFO,params);
}

void SplitByComas(string &source,vector<string> &result,char split)
{
	if(source.empty())
		return;
	const char* data = source.c_str();
	const char* tmp = data;
	while(*data != '\0')
	{
		if(*data == split)
		{
			string t(tmp,data - tmp);
			result.push_back(t);
			tmp = ++data;
			continue;
		}
		data++;
	}
//	if((*tmp != '\0') && (tmp + 1 != data))
	if((*tmp != '\0'))
	{
		string t(tmp,data - tmp);
		result.push_back(t);
	}
}

bool InitEnv(CVirtualHost **virtualHost,string &userName,char *category)
{
	string error;
	(*virtualHost) = CVirtualHost::GetVirtualHost(userName);
	if((*virtualHost) == NULL)
	{
		error.append("the file is using:");
		error.append(userName);
		WriteLog(category,ERROR,error.c_str());
		return false;
	}

	if(!(*virtualHost)->LoadFile())
	{
		error = (*virtualHost)->GetLastErrorStr();
		WriteLog(category,ERROR,error.c_str());
		return false;
	}
	return true;
}

bool ValidateParamEmpty(const char* value)
{
	if(strlen(value) == 0)
	{
		return false;
	}
	const char *tmp = value;
	while(*tmp != '\0')
	{
		if(*tmp != 32 && *tmp != 9)
			return true;
		tmp++;
	}
	return false;
}

string trim(string &str)
{
	const char* tmp = str.c_str();
	for(;*tmp != '\0';tmp++)
	{
		if(*tmp != 32 && *tmp !=9)
			break;
	}
	if(*tmp == '\0')
	{
		return "";
	}
	else
	{
		const char* end = str.c_str() + str.length() - 1;
		for(;end >= tmp;end--)
		{
			if(*end != 32 && *end != 9)
				break;
		}
		string retValue(tmp,end - tmp + 1);
		return retValue;
	}
	return "";
}

string GetValue(string key,vector<pair<string,string> > &vt_param)
{
	int size = vt_param.size();
	for(int i = 0; i < size; i++)
	{
		if(vt_param[i].first.compare(key) == 0)
		{
			return trim(vt_param[i].second);
			break;
		}
	}
	return "";
}

string MakePath(string &path,string file)
{
	AddSlash(path);
	if(file.c_str()[0] == '/')
	{
		path.append(file.c_str() + 1);
	}
	else
	{
		path.append(file);
	}
	return path;
}

string MakeUserRoot(string &userName)
{
	string path = GetEnvVar("USER_ROOT");
	if(path.empty())
	{
		path = USER_ROOT;
	}
	MakePath(path,userName);
	MakePath(path,"/home/wwwroot");
	return path;
}

string GetMaxTrans(string &max_trans)
{
	vector<string> vt;
	if(!ReadFile(&vt,"/usr/local/apache_conf/cfg/maxtrans.conf"))
		return "";
	int size = vt.size();
	vector<string> tmp;
	string max = "";
	string defaultMax;
	int i = 0;
	for(; i < size; i++)
	{
		tmp.clear();
		SplitByComas(vt[i],tmp,':');
		if(tmp.size() < 2)
			continue;
		if(strcmp(max_trans.c_str(),tmp[0].c_str()) == 0)
		{
			max = tmp[1];
			break;
		}
		if(strcmp(tmp[0].c_str(),"0") == 0)
		{
			defaultMax = tmp[1];
		}
	}
	if(i == size)
	{
		if(defaultMax.empty())
			return "";
		else
			max = defaultMax;
	}
	size_t pos = max.rfind(NEWLINE);
	if(pos != string::npos && pos == (max.size() - strlen(NEWLINE)))
	{
		max = max.substr(0,max.size() - strlen(NEWLINE));
	}
	max_trans = max;

	return max;
}
