/*************************************************************************
    > File Name: action.cpp
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年09月29日 星期日 13时27分09秒
 ************************************************************************/

#include "action.h"
#include "common.h"
#include "defines.h"
#include "tools.h"
#include <string.h>
#include <stdlib.h>
#include "config.h"
#include "log.h"
#include <stdio.h>

zlog_category_t *errorDocument = NULL;
zlog_category_t *filePermission = NULL;
zlog_category_t *deleteDirectory = NULL;
extern pthread_mutex_t mutex;

bool CAction::ProcErrorDocument(vector<pair<string,string> > &vt_param,string &errInfo)
{
	pthread_mutex_lock(&mutex);
	if(!errorDocument)
		errorDocument = GetCategory("404");
	pthread_mutex_unlock(&mutex);

	WriteParam(errorDocument,vt_param,"");

	if(vt_param.size() < 5)
	{
		errInfo.append("too less param.");
		return false;
	}
	string errorNum = "";
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
			errorNum = (*it).second;
			continue;
		}
	}
	if(errorNum.empty() || userName.empty())
	{
		errInfo.append("errorNum or ftpName not valid.");
		WriteLog(errorDocument,ERROR,"errorNum or ftpName not valid.");
		return false;
	}

	if(strcmp(errorNum.c_str(),"404") == 0 && errorPage.empty())
	{
		errorPage = "/error/404.html";
	}
	CVirtualHost* virtualHost = CVirtualHost::GetVirtualHost(userName);
	if(virtualHost == NULL)
	{
		errInfo.append("the file is using:");
		errInfo.append(userName);
		char err[256];
		sprintf(err,"the config file is using %s.conf",userName);
		WriteLog(errorDocument,ERROR,err);
		return false;
	}

	bool success = true;
	if(!BakConf(userName))
	{
		errInfo.append("bak the config file failed:");
		errInfo.append(userName);
		char err[256];
		sprintf(err,"backup the config file failed:%s.conf",userName);
		WriteLog(errorDocument,ERROR,err);
		success = false;
	}

	if(success && !virtualHost->LoadFile())
	{
		string err = virtualHost->GetLastErrorStr();
		errInfo.append(err);
		char tmp[256];
		strcpy(tmp,err.c_str());
		WriteLog(errorDocument,ERROR,tmp);
		success = false;
	}
	if(success)
	{
		string par[2] = {errorNum,errorPage};
		vector<string> vt_tmp;
		vt_tmp.push_back(errorNum);
		string directive = ERRORHEAD;
		vector<string>::iterator it_directive = virtualHost->FindGlobalDirective(directive,par,1,virtualHost->GetIterator());
		if(it_directive != virtualHost->GetEndIterator())
		{
			char tmp[256];
			strcpy(tmp,(*it_directive).c_str());
			WriteLog(errorDocument,INFO,tmp);
			it_directive = virtualHost->EraseItem(it_directive);
		}
		else
		{
			it_directive = virtualHost->GetIterator(1);
		}
		vt_tmp.push_back(errorPage);
		virtualHost->AddDirective(directive,it_directive,vt_tmp,4);
		if(!virtualHost->SaveFile())
		{
			success = false;
			string err = virtualHost->GetLastErrorStr();
			errInfo.append(err);
			char tmp[256];
			strcpy(tmp,err.c_str());
			WriteLog(errorDocument,ERROR,tmp);
			if(RestoreConf(userName))
			{
				errInfo.append("restore the config file failed:");
				errInfo.append(userName);
				char tmp[256];
				sprintf(tmp,"restore the config file failed:%s.conf",userName);
				WriteLog(errorDocument,ERROR,tmp);
			}
		}
	}
	WriteParam(errorDocument,vt_param,"success");
	CVirtualHost::ReleaseVirtualHost(userName);
	return success;
}

bool CAction::ProcFilePermission(vector<pair<string,string> > &vt_param,string &errInfo)
{	
	pthread_mutex_lock(&mutex);
	if(!filePermission)
		filePermission = GetCategory("filePermission");
	pthread_mutex_unlock(&mutex);

	WriteParam(filePermission,vt_param,"");

	if(vt_param.size() < 4)
	{
		errInfo.append("too less param.");
		return false;
	}
	int permission = -1;
	string directory = "";
	string userName = "";
	string file = "";

	//解析参数，分别赋值
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
		if((*it).first.compare(DIRECTORY) == 0)
		{
			directory = (*it).second;
			continue;
		}
	}
	//匹配目录下所有文件
	file = "\"\\w*\"";
	if(userName.empty() || file.empty() || permission < 0 || permission > 1)
	{
		errInfo.append("ftpName or permission not valid");
		WriteLog(filePermission,ERROR,"ftpName or file permission invalid");
		return false;
	}

	//获取对应的虚拟主机操作对象
	CVirtualHost* virtualHost = CVirtualHost::GetVirtualHost(userName);
	if(virtualHost == NULL)
	{
		errInfo.append("the file is using:");
		errInfo.append(userName);
		char err[256];
		sprintf(err,"the config file is using %s.conf",userName);
		WriteLog(filePermission,ERROR,err);
		return false;
	}

	bool success = true;
	if(!BakConf(userName))
	{
		errInfo.append("bak the config file failed:");
		errInfo.append(userName);
		char err[256];
		sprintf(err,"bakup the config file failed:%s.conf",userName);
		WriteLog(filePermission,ERROR,err);
		success = false;
	}
	if(success && !virtualHost->LoadFile())
	{
		string err = virtualHost->GetLastErrorStr();
		errInfo.append(err);
		success = false;
	}

	if(success)
	{
		string directive;
		string nodeName = FILENODE;
		vector<string> vt_tmpParam;
		vt_tmpParam.push_back("~");
		vt_tmpParam.push_back(file);
		vector<string>::iterator it_node = virtualHost->FindNode(nodeName,vt_tmpParam,virtualHost->GetIterator());
		vt_tmpParam.clear();
		if(it_node != virtualHost->GetEndIterator())
		{
			vector<string>::iterator it_tmp;
			vector<string> vt_tmpParam;
			directive = ORDER;
			//查找并删除Order,allow deny指令
			it_tmp = virtualHost->FindNodeDirective(it_node,directive,vt_tmpParam);
			if(it_tmp != virtualHost->GetEndIterator() && !virtualHost->IsNodeEnd((*it_tmp).c_str()))
				virtualHost->EraseItem(it_tmp);
			directive = DENY;
			it_tmp = virtualHost->FindNodeDirective(it_node,directive,vt_tmpParam);
			if(it_tmp != virtualHost->GetEndIterator() && !virtualHost->IsNodeEnd((*it_tmp).c_str()))
				virtualHost->EraseItem(it_tmp);
			directive = ALLOW;
			it_tmp = virtualHost->FindNodeDirective(it_node,directive,vt_tmpParam);
			if(it_tmp != virtualHost->GetEndIterator() && !virtualHost->IsNodeEnd((*it_tmp).c_str()))
				virtualHost->EraseItem(it_tmp);
			it_node++;
		}
		else
		{
			it_node = virtualHost->GetIterator(1);
			vt_tmpParam.push_back("~");
			vt_tmpParam.push_back(file);
			it_node = virtualHost->AddNode(nodeName,it_node,vt_tmpParam);
		}
		
		vt_tmpParam.clear();
		switch(permission)
		{
			case PERMISSION_ALLOW:		
			{
				vt_tmpParam.push_back("deny,allow");
				directive = ORDER;
				virtualHost->AddDirective(directive,it_node,vt_tmpParam,8);
				it_node++;
				vt_tmpParam.clear();
				vt_tmpParam.push_back("from");
				vt_tmpParam.push_back("all");
				directive = ALLOW;
				virtualHost->AddDirective(directive,it_node,vt_tmpParam,8);
				break;
			}
			case PERMISSION_DENY:
			{
				vt_tmpParam.push_back("allow,deny");
				directive = ORDER;
				virtualHost->AddDirective(directive,it_node,vt_tmpParam,8);
				it_node++;
				vt_tmpParam.clear();
				vt_tmpParam.push_back("from");
				vt_tmpParam.push_back("all");
				directive = DENY;
				virtualHost->AddDirective(directive,it_node,vt_tmpParam,8);
				break;
			}
		}
		
		if(!virtualHost->SaveFile())
		{
			success = false;
			string err = virtualHost->GetLastErrorStr();
			errInfo.append(err);
			char tmp[256];
			strcpy(tmp,err.c_str());
			WriteLog(filePermission,ERROR,tmp);
			if(RestoreConf(userName))
			{
				errInfo.append("restore the config file failed:");
				errInfo.append(userName);
				char tmp[256];
				sprintf(tmp,"restore the config file failed:%s.conf",userName);
				WriteLog(filePermission,ERROR,tmp);
			}
		}
	}
	CVirtualHost::ReleaseVirtualHost(userName);
	WriteParam(filePermission,vt_param,"success");
	return success;
}


void CAction::DeleteRootDirectory(vector<pair<string,string> >&vt_param,string &errInfo)
{
	pthread_mutex_lock(&mutex);
	if(!deleteDirectory)
		deleteDirectory = GetCategory("deleteDirectory");
	pthread_mutex_unlock(&mutex);

	WriteParam(deleteDirectory,vt_param,"");
	if(vt_param.size() < 3)
	{
		errInfo.append("too less param.");
		WriteParam(deleteDirectory,vt_param,"failed");
		return;
	}

	string userName = vt_param[2].second;
	Config config;
	config.LoadConfigFile();
	string path = config.GetValue("USER_ROOT");
	if(path.empty())
	{
		path = "/var/www/virtual/";
	}
	path.append(userName);
	path.append("/home/wwwroot");
	RmDir(path.c_str());
	WriteParam(deleteDirectory,vt_param,"success");
}


