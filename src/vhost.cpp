/*************************************************************************
    > File Name: vhost.cpp
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年11月07日 星期四 16时46分41秒
 ************************************************************************/

#include "vhost.h"
#include "log.h"
#include "common.h"
#include "defines.h"
#include "tools.h"
#include "passwd.h"
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <stdlib.h>
#include <pwd.h>

using namespace std;

static char log[] = "vhost";
void Replace(vector<pair<string,string> > &vt_param, vector<string> &vt)
{
	vector<string>::iterator it = vt.begin();
	int size = vt_param.size();
	size_t pos;
	for(; it != vt.end(); it++)
	{
		for(int i = 0; i < size; i++)
		{
			pos = (*it).find(vt_param[i].first);
			while(pos != string::npos)
			{
				(*it).replace(pos,strlen(vt_param[i].first.c_str()),vt_param[i].second);
				pos = (*it).find(vt_param[i].first);
			}
		}
	}
}

bool CreateUser(string &userName)
{
	string path = GetEnvVar("USER_ROOT");
	if(path.empty())
		path = USER_ROOT;

	MakePath(path,userName);
	if(mkdir(path.c_str(),0) != 0)
	{
		char info[] = "failed to create the root directory.";
		WriteLog(log,ERROR,info);
		return false;
	}

	string supe_home = GetEnvVar("SUPE_HOME");
	if(supe_home.empty())
	{
		char info[] = "please set the SUPE_HOME directory.";
		WriteLog(log,ERROR,info);
		return false;
	}

	MakePath(supe_home,"/*");
	
	char cmd[256];
	if(snprintf(cmd,256,"cp -a %s %s",supe_home.c_str(),path.c_str()) >= 256)
	{
		char info[] = "the shell cp ... is too long.";
		WriteLog(log,ERROR,info);
		return false;
	}
	int ret = system(cmd);

	string user_shell = GetEnvVar("USER_SHELL");
	if(user_shell.empty())
	{
		user_shell = USER_SHELL;
	}
	MakePath(path,"/home");

	if(snprintf(cmd,256,"/usr/sbin/useradd -d %s -m -s %s %s",path.c_str(),user_shell.c_str(),userName.c_str()) >= 256)
	{

		char info[] = "the shell useradd ... is too long.";
		WriteLog(log,ERROR,info);
		return false;
	}
	ret = system(cmd);

	if(ret != -1 && WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
		return true;
	else
	{
		char info[] = "failed to create the user.";
		WriteLog(log,ERROR,info);
		return false;
	}
}

bool SetConfiguration(string &userName)
{
	string path = GetEnvVar("USER_ROOT");
	MakePath(path,userName);
	string cgi_path = path;
	string ini_path = path;
	
	MakePath(cgi_path,"/bin/php-cgi");
	MakePath(ini_path,"/home/php.ini");

	vector<string> vt;
	vector<pair<string,string> > param;
	if(!ReadFile(&vt,"/usr/local/apache_conf/tpl/php-cgi.sample"))
	{
		
		char info[] = "failed to read php-cgi.sample file.";
		WriteLog(log,ERROR,info);
		return false;
	}
	pair<string,string> p;
	p.first = "{ftpuser}";
	p.second = userName;
	param.push_back(p);

	Replace(param,vt);
	if(!WriteFile(&vt,cgi_path.c_str()))
	{
		char info[] = "failed to write php-cgi file.";
		WriteLog(log,ERROR,info);
		return false;
	}

	vt.clear();
	if(!ReadFile(&vt,"/usr/local/apache_conf/tpl/php.ini.sample"))
	{
		char info[] = "failed to read php.ini.sample file.";
		WriteLog(log,ERROR,info);
		return false;
	}
	Replace(param,vt);
	if(!WriteFile(&vt,ini_path.c_str()))
	{
		char info[] = "failed to write php.ini.sample file.";
		WriteLog(log,ERROR,info);
		return false;
	}
	return true;
}

bool SetDirPer(string root,string dir,mode_t mode)
{
	if(!dir.empty())
		MakePath(root,dir);
	if(chmod(root.c_str(),mode) != 0)
	{
		string info = "failed to set the directory permission:";
		info.append(root);
		WriteLog(log,ERROR,info.c_str());
		return false;
	}
	return true;
}

bool ChangeOwner(const char *path,string &userName)
{
	char cmd[PATH_MAX];
	if(snprintf(cmd,PATH_MAX,"chown -R %s:%s %s",userName.c_str(),userName.c_str(),path) >= PATH_MAX)
	{
		WriteLog(log,ERROR,"chorn -R is too long.");
		return false;
	}
	int ret = system(cmd);
	if(ret != -1 && WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
		return true;
	else
	{
		strcat(cmd," failed");
		WriteLog(log,ERROR,cmd);
		return false;
	}
}

extern pthread_mutex_t mutex; 
bool SetPermission(string &userName)
{
	//改变所有者
	string user_root = GetEnvVar("USER_ROOT");
	if(user_root.empty())
	{
		user_root = USER_ROOT;
	}
	MakePath(user_root,userName);

	if(!ChangeOwner(user_root.c_str(),userName))
	{
		return false;
	}

	//改变权限
	mode_t mode = S_IRUSR | S_IWUSR | S_IXUSR  | S_IXGRP | S_IXOTH;
	if(!SetDirPer(user_root,"",mode) || !SetDirPer(user_root,"/bin",mode) || !SetDirPer(user_root,"/home",mode) || !SetDirPer(user_root,"/home/cgi-bin",mode) || !SetDirPer(user_root,"/home/wwwroot",mode))
	{
		return false;
	}
	
	mode = S_IRUSR | S_IWUSR | S_IXUSR;
	if(!SetDirPer(user_root,"/session",mode))
	{
		return false;
	}
	mode = S_IRUSR | S_IXUSR;
	if(!SetDirPer(user_root,"/bin/php-cgi",mode))
	{
		return false;
	}
	pthread_mutex_lock(&mutex);
	struct passwd *pwd;
	pwd = getpwnam("apache");
	if(pwd == NULL)
	{
		pthread_mutex_unlock(&mutex);
		return false;
	}
	uid_t apache_uid = pwd->pw_uid;

	pwd = getpwnam("root");
	if(pwd == NULL)
	{
		pthread_mutex_unlock(&mutex);
		return false;
	}
	gid_t root_git = pwd->pw_gid;
	pthread_mutex_unlock(&mutex);

	string log_path = user_root;
	MakePath(log_path,"/home/logs");
	if(chown(log_path.c_str(),apache_uid,root_git) != 0)
	{
		return false;
	}
	
	mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IWGRP | S_IROTH | S_IXOTH;
	if(!SetDirPer(user_root,"/home/logs",775))
	{
		return false;
	}

	mode = S_IRUSR | S_IWUSR;
	if(!SetDirPer(user_root,"/home/php.ini",600))
	{
		return false;
	}
	return true;
}

bool WriteConfFile(string &userName,string &defaultdomain,string &max_trans)
{
	vector<string> vt;
	vector<pair<string,string> > param;
	
	if(!ReadFile(&vt,"/usr/local/apache_conf/tpl/vhost-conf.sample"))
	{
		char info[] = "read the config sample file failed.";
		WriteLog(log,ERROR,info);
		return false;
	}

	pair<string,string> p;
	p.first = "{ftpuser}";
	p.second = userName;
	param.push_back(p);

	p.first = "{defaultdomain}";
	p.second = defaultdomain;
	param.push_back(p);

	p.first = "{max_trans}";
	p.second = max_trans;
	param.push_back(p);
	
	Replace(param,vt);
	string path = MakeConfPath(userName);	
	if(!WriteFile(&vt,path.c_str()))
	{
		char info[] = "write the config file failed.";
		WriteLog(log,ERROR,info);
		return false;
	}
	return true;
}

bool SetQuota(string &userName,string &max_trans)
{
	long long quota = strtoll(max_trans.c_str(),NULL,10) * 1024;
	long long hard_quota = quota + 10240;
	char cmd[256];
	string user_root = GetEnvVar("USER_ROOT");
	if(user_root.empty())
	{
		user_root = USER_ROOT;
	}
	if(snprintf(cmd,256,"/usr/sbin/setquota -u %s %lld %lld 0 0 %s",userName.c_str(),quota,hard_quota,user_root.c_str()) >= 256)
	{
		return false;
	}

	int ret = system(cmd);
	if(ret != -1 && WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
	{
		return true;
	}
	else
	{
		char info[] = "set the quota failed.";
		WriteLog(log,ERROR,info);
		return false;
	}
}

bool InstallFiles(string &userName)
{
	string path = GetEnvVar("USER_ROOT");
	if(path.empty())
		path = USER_ROOT;

	MakePath(path,userName);
	MakePath(path,"/home/wwwroot");

	string zipName = "/usr/local/apache_conf/installfiles/shopex2.zip";

	char cmd[256];
	if(snprintf(cmd,256,"unzip -q %s -d %s > /dev/null",zipName.c_str(),path.c_str()) >= 256)
	{
		return false;
	}

	int ret = system(cmd);
	if(ret != -1 && WIFEXITED(ret) && (WEXITSTATUS(ret) == 0 || WEXITSTATUS(ret) == 12))
	{
		pthread_mutex_lock(&mutex);
		struct passwd *pwd;
		pwd = getpwnam(userName.c_str());
		if(pwd == NULL)
		{
			pthread_mutex_unlock(&mutex);
			return false;
		}
		uid_t uid = pwd->pw_uid;
		gid_t gid = pwd->pw_gid;
		pthread_mutex_unlock(&mutex);
		//改变所有者
		if(!ChangeOwner(path.c_str(),userName))
		{
			return false;
		}
	}
	else
	{
		return false;
	}
	return true;
}

bool DeleteUser(string &userName)
{
	char cmd[PATH_MAX];
	sprintf(cmd,"/usr/sbin/userdel %s",userName.c_str());
	int ret = system(cmd);

	if(ret != -1 && WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
	{
		return true;
	}
	else
		return false;
}

bool DeleteConf(string &userName)
{
	string path = MakeConfPath(userName);
	if(unlink(path.c_str()) != 0)
	{
		return false;
	}
	else
		return true;
}

bool Clean(string &userName)
{
	DeleteUser(userName);
	char cmd[PATH_MAX];
	string path = GetEnvVar("USER_ROOT");
	if(path.empty())
		path = USER_ROOT;
	MakePath(path,userName);

	if(snprintf(cmd,PATH_MAX,"rm -rf %s",path.c_str()) >= PATH_MAX)
	{
		;
	}
	int ret = system(cmd);
	
	if(ret != -1 && WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
	{
		return true;
	}
	else
		return false;
}

bool UserExist(string &userName)
{
	pthread_mutex_lock(&mutex);
	setpwent();
	struct passwd* pwd;
	bool exist = false;
	while((pwd = getpwent()) != NULL)
	{
		if(strcmp(pwd->pw_name,userName.c_str()) == 0)
		{
			exist = true;
			break;
		}
	}
	endpwent();
	pthread_mutex_unlock(&mutex);
	return exist;
}

bool CreateVHost(vector<pair<string,string> > &vt_param,string &error)
{
	WriteParam(log,vt_param,"");

	string userName = GetValue(USERNAME,vt_param);
	string pwd = GetValue(PWD,vt_param);
	string defaultDomain = GetValue(DOMAIN,vt_param);
	string max_trans = GetValue(MAX_TRANS,vt_param);

	if(!ValidateParamEmpty(userName.c_str()) || !ValidateParamEmpty(pwd.c_str()) || !ValidateParamEmpty(defaultDomain.c_str()))
	{
		char info[] = "ftpName or pwd or domain invalid.";
		WriteLog(log,ERROR,info);
		return false;
	}
	
	//添加系统用户
	//判断用户是否存在
	if(UserExist(userName))
	{
		error.append("user exists.");
		WriteLog(log,ERROR,"user exists");
		return false;
	}
	if(!CreateUser(userName))
	{
		Clean(userName);
		return false;
	}
	//写php-cgi和php.ini文件
	if(!SetConfiguration(userName))
	{
		Clean(userName);
		return false;
	}
	//设置权限
	if(!SetPermission(userName))
	{
		Clean(userName);
		return false;
	}

	//安装文件
	if(!InstallFiles(userName))
	{
		Clean(userName);
		return false;
	}

	//设置密码
	string tmp;
	if(!ProcPasswd(vt_param,tmp))
	{
		Clean(userName);
		return false;
	}

	//设置磁盘份额
	if(!SetQuota(userName,max_trans))
	{
		Clean(userName);
		return false;
	}

	//获取连接数
	max_trans = GetMaxTrans(max_trans);
	if(max_trans.empty())
	{
		Clean(userName);
		return false;
	}

	//写入配置文件
	if(!WriteConfFile(userName,defaultDomain,max_trans))
	{
		Clean(userName);
		return false;
	}
	//备份文件
	BakSysInfo();
	return true;
}

bool DeleteVHost(vector<pair<string,string> > &vt_param,string &error)
{
	WriteParam(log,vt_param,"");
	
	string userName = GetValue(USERNAME,vt_param);
	if(!ValidateParamEmpty(userName.c_str()))
	{
		char info[] = "ftpName invalid.";
		WriteLog(log,ERROR,info);
		return false;
	}

	if(!DeleteConf(userName))
	{
		char info[] = "delete the config file failed.";
		WriteLog(log,ERROR,info);
	}
	BakConf();

	if(!DeleteUser(userName))
	{
		char info[] = "delete the user failed.";
		WriteLog(log,ERROR,info);
	}
	BakSysInfo();

	string path = GetEnvVar("USER_ROOT");
	if(path.empty())
	{
		path = USER_ROOT;
	}
	MakePath(path,userName);
	
	string owner = "root";
	ChangeOwner(path.c_str(),owner);
	
	time_t t = time(NULL);
	struct tm *local = gmtime(&t);
	char cmd[PATH_MAX];
	if(snprintf(cmd,PATH_MAX,"mv /var/www/virtual/%s /var/www/virtual/%s.%d%d%d.drop",userName.c_str(),userName.c_str(),local->tm_year + 1900 ,local->tm_mon + 1,local->tm_mday) >= PATH_MAX)
	{
		char info[] = "the mv command is too long.";
		WriteLog(log,ERROR,info);
		return false;
	}
	int ret = system(cmd);
	if(ret != -1 && WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
	{
		return true;
	}
	else
	{
		char info[] = "rename the root directory failed.";
		WriteLog(log,ERROR,info);
		return false;
	}
}
