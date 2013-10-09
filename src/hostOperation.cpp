/*************************************************************************
    > File Name: hostOperation.cpp
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年09月16日 星期一 10时14分49秒
 ************************************************************************/

#include "hostOperation.h"
#include "tools.h"
#include "action.h"
#include <syslog.h>
#include <stdlib.h>
#include "log.h"

bool ProcHost(vector<pair<string,string> > vt_param,string &errInfo)
{
	if(vt_param.size() < 2)
	{
		errInfo.append("too less param. ");
		return false;
	}
	pair<string,string> p = vt_param[1];
	if(!IsEqualString(p.first,NFUNC))
	{
		errInfo.append("error param:nfunc. ");
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
		if(!CAction::ProcErrorDocument(vt_param,errInfo))
			return false;
	}
	else if(IsEqualString(value,FILEPERMISSION))
	{
		//脚本权限
		if(!CAction::ProcFilePermission(vt_param,errInfo))
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
			errInfo.append("ftpName cant't be empty ");
			return false;
		}
		else
		{
			if(!RestoreConf(ftpName))
			{
				errInfo.append("failed to restore the config filei:");
				errInfo.append(ftpName);
				errInfo.append(". ");
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
	else if(IsEqualString(value,DELETEDIR))
	{
		CAction::DeleteRootDirectory(vt_param,errInfo);
	}
	else
	{
		errInfo.append("unknow operation:");
		errInfo.append(value);
		errInfo.append(". ");
	}
	int ret = system("/sbin/service httpd restart >> /dev/null");
	
	if(ret != -1 && WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
	{
		//WriteLog(INFO,"restart apache success.");
	}
	else
	{
		errInfo.append("failed to restart apache. ");
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
