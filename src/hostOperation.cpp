/*************************************************************************
    > File Name: hostOperation.cpp
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年09月16日 星期一 10时14分49秒
 ************************************************************************/

#include "hostOperation.h"
#include "ftp.h"
#include <string.h>
#include <syslog.h>
#include "tools.h"

bool ProcErrorDocument(vector<pair<string,string> > &vt_param,string &errInfo);
bool ProcFilePermission(vector<pair<string,string> > &vt_param,string &errInfo);
//bool AddHost(vector<pair<string,string> > &vt_param);

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

bool ProcHost(vector<pair<string,string> > vt_param,string &errInfo)
{
	if(vt_param.size() < 2)
	{
		errInfo.append("参数太少");
		errInfo.append(SPLIT);
		return false;
	}
	pair<string,string> p = vt_param[1];
	if(IsEqualString(p.first,NFUNC))
	{
		errInfo.append("参数错误，未指定操作类型");
		errInfo.append(SPLIT);
		return false;
	}
	string value = p.second;
	if(IsEqualString(value,CREATEHOST))
	{
		//添加虚拟主机操作
	/*	if(!AddHost(vt_param))
			return false;*/
	}
	else if(IsEqualString(value,ERRORDOCUMENT))
	{
		//修改错误页面
		if(!ProcErrorDocument(vt_param,errInfo))
			return false;
	}
	else if(IsEqualString(value,FILEPERMISSION))
	{
		//脚本权限
		if(!ProcFilePermission(vt_param,errInfo))
			return false;
	}

	else if(IsEqualString(value,RESTORECONF))
	{
		string ftpName = "";
		for(int i = 2; i < vt_param.size(); i++)
		{
			if(IsEqualString(vt_param[i].first,USERNAME))
			{
				ftpName = vt_param[i].second;
				break;
			}
		}

		if(ftpName.empty())
		{
			errInfo.append("ftpName不能为空");
			errInfo.append(SPLIT);
			return false;
		}
		else
		{
			if(!RestoreConf(ftpName))
			{
				errInfo.append("恢复配置文件失败：");
				errInfo.append(ftpName);
				errInfo.append(SPLIT);
				return false;
			}
		}
	}
	else if(IsEqualString(value,MYSQLBACK))
	{
		return MySQLBack(vt_param,errInfo);
	}
	else if(IsEqualString(value,MYSQLRESTORE))
	{
		return MySQLRestore(vt_param,errInfo);
	}
	else
	{
		errInfo.append("未知的操作类型:");
		errInfo.append(value);
		errInfo.append(SPLIT);
	}
	if(!UpLoadFile())
	{
		//上传文件错误
		
	}
	int ret = system("/sbin/service httpd restart >> /dev/null");
	
	if(ret != -1 && WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
	{
		syslog(LOG_INFO,"重启apache成功");
	}
	else
	{
		errInfo.append("重启apache失败");
		errInfo.append(SPLIT);
		return false;
	}
	return true;
}

bool ProcErrorDocument(vector<pair<string,string> > &vt_param,string &errInfo)
{
	if(vt_param.size() < 5)
	{
		errInfo.append("参数太少");
		errInfo.append(SPLIT);
		return false;
	}
	int errorNum = -1;
	string errorPage = "";
	string userName = "";
	vector<pair<string,string> >::iterator it = vt_param.begin();
	it++;
	it++;
	for(;it != vt_param.end(); it++)
	{
		if((*it).first.compare(ERRORPAGE) == 0)
		{
			errorPage = (*it).second;
			continue;
		}
		if((*it).first.compare(USERNAME) == 0)
		{
			userName = (*it).second;
			continue;
		}
		if((*it).first.compare(ERRORNMSTR) == 0)
		{
			errorNum = atoi((*it).second.c_str());
			continue;
		}
	}
	if(errorNum <0 || errorPage.empty() || userName.empty())
	{
		errInfo.append("errorNum或errorPage或ftpName不合法");
		errInfo.append(SPLIT);
		return false;
	}

	if(!BakConf(userName))
	{
		errInfo.append("备份配置文件失败:");
		errInfo.append(userName);
		errInfo.append(SPLIT);
		return false;
	}
	string conf_path = MakeConfPath(userName);

	vector<string> *vt_conf = new vector<string>();

	if(!ReadFile(vt_conf,conf_path.c_str()))
	{
		//读取文件出错
		errInfo.append("读取配置文件出错:");
		errInfo.append(userName);
		errInfo.append(SPLIT);
		return false;
	}
	ChangeError(errorNum,errorPage,vt_conf);
	if(!WriteFile(vt_conf,conf_path.c_str()))
	{
		//写文件出错
		errInfo.append("写入配置文件出错:");
		errInfo.append(userName);
		errInfo.append(SPLIT);
		//恢复配置
		if(!RestoreConf(userName))
		{
			errInfo.append("恢复配置文件失败:");
			errInfo.append(userName);
			errInfo.append(SPLIT);
		}
		return false;
	}
	delete vt_conf;
	return true;
}

bool ProcFilePermission(vector<pair<string,string> > &vt_param,string &errInfo)
{	
	if(vt_param.size() < 4)
	{
		errInfo.append("参数太少");
		errInfo.append(SPLIT);
		return false;
	}
	int permission = -1;
	string directory = "";
	string userName = "";
	string file = "";

	vector<pair<string,string> >::iterator it = vt_param.begin();
	it++;
	it++;
	for(;it != vt_param.end(); it++)
	{
		if((*it).first.compare(USERNAME) == 0)
		{
			userName = (*it).second;
			continue;
		}
		if((*it).first.compare(PERMISSION) == 0)
		{
			permission = atoi((*it).second.c_str());
			continue;
		}
	}
	file = "\\w*";
	if(userName.empty() || file.empty() || permission < 0 || permission > 1)
	{
		errInfo.append("ftpName或permission不合法");
		errInfo.append(SPLIT);
		return false;
	}
	
	if(!BakConf(userName))
	{
		errInfo.append("备份配置文件失败:");
		errInfo.append(userName);
		errInfo.append(SPLIT);
		return false;
	}
	string path = MakeConfPath(userName);
	vector<string> vt_conf;

	if(!ReadFile(&vt_conf,path.c_str()))
	{
		//读取文件出错
		errInfo.append("读取配置文件出错:");
		errInfo.append(userName);
		errInfo.append(SPLIT);
		return false;
	}
	ChangePermission(directory,file,permission,vt_conf);
	if(!WriteFile(&vt_conf,path.c_str()))
	{
		//写入文件出错
		errInfo.append("写入配置文件出错:");
		errInfo.append(userName);
		errInfo.append(SPLIT);
		//恢复配置
		if(!RestoreConf(userName))
		{
			errInfo.append("恢复配置文件失败:");
			errInfo.append(userName);
			errInfo.append(SPLIT);
		}
		return false;
	}
	return true;
}

/*bool AddHost(vector<pair<string,string> > &vt_param)
{
	if(vt_param.size() < 6)
		return false;
	string userName = "";
	string defaultDomain = "";
	string maxTrans = "";
	string maxConn = "";

	vector<pair<string,string> >::iterator it = vt_param.begin();
	it++;
	it++;
	for(;it != vt_param.end(); it++)
	{
		if((*it).first.compare(DEFAULTDOMAIN) == 0)
		{
			defaultDomain = (*it).second;
			continue;
		}
		if((*it).first.compare(USERNAME) == 0)
		{
			userName = (*it).second;
			continue;
		}
		if((*it).first.compare(MAXTRANS) == 0)
		{
			maxTrans = atoi((*it).second.c_str());
			continue;
		}
		if((*it).first.compare(MAXCONN) == 0)
		{
			maxConn = (*it).second;
			continue;
		}
	}
	
	if(userName.empty() || maxTrans.empty() || defaultDomain.empty() || maxConn.empty())
		return false;

	//创建用户所需的各种目录
	string dir = USER_ROOT;
	dir.append(userName);
	int ret = mkdir(dir.c_str(),0x000);
	if(ret != 0)
	{
		//创建目录失败
		return false;
	}
//	chdir(dir.c_str());
//	mkdir("bin");
//	mkdir("home");
//	mkdir("");
	string cpcmd = "cp -a ";
	cpcmd.append(SUPE_HOME);
	cpcmd.append("/* ");
	cpcmd.append(dir);
	ret = system(cpcmd.c_str());

	string usercmd = "/usr/sbin/useradd -d ";
	usercmd.append(dir);
	usercmd.append("/home -m -s ");
	usercmd.append(USER_SHELL);
	usercmd.append(" ");
	usercmd.append(userName);
	ret = system(usercmd.c_str());



	CallPython c;
	RunScript("import sys");
	RunScript("sys.path.append('./')");
	if(!c.LoadFile("replace"))
	{
		return false;
	}
	if(!c.GetFunc("doReplaceCGIFile"))
	{
		return false;
	}
	string cgipath = dir;
	dir.append("/bin/php-cgi");
	string cgiparam[]={CGI_TPL,cgipath,userName};
	c.BuildArgs(3,cgiparam);
	c.Invoke();

	if(!c.GetFunc("doReplacePhpFile"))
	{
		return false;
	}
	string phppath = dir;
	phppath.append("/home/php.ini");
	string phpparam[]={PHPINI_TPL,phppath,userName};
	c.BuildArgs(3,phpparam);
	c.Invoke();

	if(!c.GetFunc("doReplaceHostFile"))
	{
		return false;
	}
	string path = MakeConfPath(userName);
	string param[] = {CONF_TPL,path,userName,defaultDomain,maxTrans,maxConn};
	c.BuildArgs(6,param);
	return c.Invoke();
}*/
