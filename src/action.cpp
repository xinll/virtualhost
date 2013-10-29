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

char *errorDocument = "404Log";
char *filePermission = "filePermissionLog";
char *deleteDirectory = "deleteDirectoryLog";
char *redirect = "redirectLog";
char *mine = "mineLog";
char *directoryAccess = "directoryAccessLog";
//extern pthread_mutex_t mutex;

bool CAction::WriteFile(CVirtualHost *virtualHost,string &errInfo,char *category)
{
	string userName = virtualHost->GetFileName();
	if(!virtualHost->SaveFile())
	{
		string err = virtualHost->GetLastErrorStr();
		errInfo.append(err);
		char tmp[256];
		strcpy(tmp,err.c_str());
		WriteLog(category,ERROR,tmp);
		if(RestoreConf(userName))
		{
			errInfo.append("restore the config file failed:");
			errInfo.append(userName);
			char tmp[256];
			sprintf(tmp,"restore the config file failed:%s.conf",userName.c_str());
			WriteLog(category,ERROR,tmp);
		}
		return false;
	}
	return true;
}
bool CAction::InitEnv(CVirtualHost **virtualHost,string &userName,string &errInfo,char *category)
{
	 (*virtualHost) = CVirtualHost::GetVirtualHost(userName);
	if((*virtualHost) == NULL)
	{
		errInfo.append("the file is using:");
		errInfo.append(userName);
		char err[256];
		sprintf(err,"the config file is using %s.conf",userName.c_str());
		WriteLog(category,ERROR,err);
		return false;
	}

	if(!BakConf(userName))
	{
		errInfo.append("bak the config file failed:");
		errInfo.append(userName);
		char err[256];

		sprintf(err,"backup the config file failed:%s.conf",userName.c_str());
		WriteLog(category,ERROR,err);

		return false;
	}

	if(!(*virtualHost)->LoadFile())
	{
		string err = (*virtualHost)->GetLastErrorStr();
		errInfo.append(err);
		char tmp[256];
		strcpy(tmp,err.c_str());
		WriteLog(category,ERROR,tmp);
		return false;
	}
	return true;
}

bool CAction::ProcErrorDocument(vector<pair<string,string> > &vt_param,string &errInfo)
{
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

	CVirtualHost *virtualHost;
	bool success = InitEnv(&virtualHost,userName,errInfo,errorDocument);

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
		success = WriteFile(virtualHost,errInfo,errorDocument);
	}
	if(success)
		WriteParam(errorDocument,vt_param,"success");
	else
		WriteParam(errorDocument,vt_param,"failed");
	CVirtualHost::ReleaseVirtualHost(userName);
	return success;
}

bool CAction::ProcFilePermission(vector<pair<string,string> > &vt_param,string &errInfo)
{	
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

	CVirtualHost *virtualHost;
	bool success = InitEnv(&virtualHost,userName,errInfo,filePermission);

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
		
		success = WriteFile(virtualHost,errInfo,filePermission);
	}
	if(success)
		WriteParam(filePermission,vt_param,"success");
	else
		WriteParam(filePermission,vt_param,"failed");
	CVirtualHost::ReleaseVirtualHost(userName);
	return success;
}


void CAction::DeleteRootDirectory(vector<pair<string,string> >&vt_param,string &errInfo)
{
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

void CAction::AddRedirect(string redirectFrom,string redirectTo,CVirtualHost *virtualHost)
{
	char buf[256];
	string directive = REWRITEENGINE;
	vector<string> vt_tmpParam;
	vector<string>::iterator it = virtualHost->FindGlobalDirective(directive,NULL,0,virtualHost->GetIterator());
	//处理rewriteengine
	bool exist = false;
	if(it != virtualHost->GetEndIterator())
	{
		exist = true;
		vector<string> vt_tmp;
		Split((*it),vt_tmp);
		if(vt_tmp.size() != 2 || vt_tmp[1].compare("on") != 0)
		{
			string log = "line delete: ";
			log.append(*it);
			sprintf(buf,"%s from %s",log.c_str(),virtualHost->GetFileName().c_str());
			WriteLog(redirect,INFO,buf);
			virtualHost->EraseItem(it);
			exist = false;
		}
		
	}
	if(!exist)
	{
		vt_tmpParam.push_back("on");
		virtualHost->AddDirective(directive,virtualHost->GetIterator() + 1,vt_tmpParam,4);
		string log = "line add: ";
		log.append(directive);
		log.append(" on");
		sprintf(buf,"%s from %s",log.c_str(),virtualHost->GetFileName().c_str());
		WriteLog(redirect,INFO,buf);
	}
	directive = "RewriteCond";
	string tmp = "^";
	tmp.append(redirectFrom);
	tmp.append("$");
	string param[] = {"%{HTTP_POST}",tmp};
	it = virtualHost->FindGlobalDirective(directive,param,2,virtualHost->GetIterator());
	if(it != virtualHost->GetEndIterator())
	{
		it++;
		string log = "line delete: ";
		log.append(*it);
		sprintf(buf,"%s from %s",log.c_str(),virtualHost->GetFileName().c_str());
		WriteLog(redirect,INFO,buf);
		virtualHost->EraseItem(it);
	}
	else
	{
		vt_tmpParam.clear();
		directive = REWRITEENGINE;
		vector<string>::iterator it_insert = virtualHost->FindGlobalDirective(directive,NULL,0,virtualHost->GetIterator());
	/*	it_insert++;
		virtualHost->AddDirective(directive,it_insert,vt_tmpParam,4);
		vt_tmpParam.clear();*/
		it_insert++;

		directive = "RewriteCond";
		vt_tmpParam.push_back("%{HTTP_POST}");
		string tmp = "^";
		tmp.append(redirectFrom);
		tmp.append("$");
		vt_tmpParam.push_back(tmp);
		vt_tmpParam.push_back("[NC]");
		virtualHost->AddDirective(directive,it_insert,vt_tmpParam,4);
		string log = "line add: ";
		log.append(*it);
		sprintf(buf,"%s from %s",log.c_str(),virtualHost->GetFileName().c_str());
		WriteLog(redirect,INFO,buf);
		vt_tmpParam.clear();
		it_insert++;
		it = it_insert;
	}
	directive="RewriteRule";
	vt_tmpParam.push_back("^(.*)$");
	redirectTo.append("$1");
	vt_tmpParam.push_back(redirectTo);
	vt_tmpParam.push_back("[L,R=301]");
	virtualHost->AddDirective(directive,it,vt_tmpParam,4);
	string log = "line add: ";
	log.append(*it);
	sprintf(buf,"%s from %s",log.c_str(),virtualHost->GetFileName().c_str());
	WriteLog(redirect,INFO,buf);
}

void CAction::DeleteRedirect(string redirectFrom,CVirtualHost *virtualHost)

{
	string directive = "RewriteCond";
	string tmp = "^";
	tmp.append(redirectFrom);
	tmp.append("$");
	string param[] = {"%{HTTP_POST}",tmp};
	vector<string>::iterator it = virtualHost->FindGlobalDirective(directive,param,2,virtualHost->GetIterator());
	if(it != virtualHost->GetEndIterator())
	{
		string log = "line delete: ";
		log.append(*it);
		char buf[256];
		sprintf(buf,"%s from %s",log.c_str(),virtualHost->GetFileName().c_str());
		WriteLog(redirect,INFO,buf);
		virtualHost->EraseItem(it);

		log = "line delete: ";
		log.append(*it);
		sprintf(buf,"%s from %s",log.c_str(),virtualHost->GetFileName().c_str());
		WriteLog(redirect,INFO,buf);
	/*	if(redirectTo.c_str()[redirectTo.size() - 1] != '/')
			redirectTo.append("/");
		redirectTo.append("$1");
		param[0] = "^(.*)$";
		param[1] = redirectTo;
		it = virtualHost->FindGlobalDirective(directive,param,2,it);
		if(it != virtualHost.GetEndIterator())*/
			virtualHost->EraseItem(it);
	}
	directive = "RewriteRule";
	it = virtualHost->FindGlobalDirective(directive,NULL,0,virtualHost->GetIterator());
	if(it == virtualHost->GetEndIterator())
	{
		directive = REWRITEENGINE;
		it = virtualHost->FindGlobalDirective(directive,NULL,0,virtualHost->GetIterator());
		if(it != virtualHost->GetEndIterator())
			virtualHost->EraseItem(it);
	}
}

bool CAction::ProcRedirect(vector<pair<string,string> > &vt_param,string &errInfo)
{
	WriteParam(redirect,vt_param,"");

	if(vt_param.size() < 4)
	{
		errInfo.append("too less param.");
		return false;
	}
	string userName;
	string url = "";

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
		if((*it).first.compare(URL) == 0)
		{
			url	= (*it).second;
			continue;
		}
	}

	if(userName.empty() || url.empty())
	{
		errInfo.append("ftpName or url not valid");
		WriteLog(redirect,ERROR,"ftpName or url invalid");
		return false;
	}

	CVirtualHost *virtualHost;
	bool success = InitEnv(&virtualHost,userName,errInfo,redirect);

	if(success)
	{
		vector<string> vt_url;
		vector<string> vt_from;
		vector<string> vt_tmp;
		string to = "";
		SplitByComas(url,vt_url,';');
		int url_size = vt_url.size();
		for(int i = 0; i < url_size; i++)
		{
			vt_tmp.clear();
			SplitByComas(vt_url[i],vt_tmp,':');
			vt_from.clear();	
			SplitByComas(vt_tmp[0],vt_from);
			if(vt_tmp.size() > 1)
				to = vt_tmp[1];
			int size = vt_from.size();
			for(int k = 0; k < size; k++)
			{
				if(!to.empty())
					AddRedirect(vt_from[k],to,virtualHost);
				else
					DeleteRedirect(vt_from[k],virtualHost);
			}
		}
	}
	if(success)
		success = WriteFile(virtualHost,errInfo,redirect);

	if(success)
		WriteParam(redirect,vt_param,"success");
	else
		WriteParam(redirect,vt_param,"failed");
	CVirtualHost::ReleaseVirtualHost(userName);
	return success;
}

bool CAction::ProcMineType(vector<pair<string,string> > &vt_param,string &errInfo)
{
	WriteParam(mine,vt_param,"");

	if(vt_param.size() < 5)
	{
		errInfo.append("too less param.");
		return false;
	}
	string userName;
	string action;
	string mineType;
	string procMethod;

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
		if((*it).first.compare(MINETYPE) == 0)
		{
			mineType = (*it).second;
			continue;
		}
		if((*it).first.compare(MINEPROCESS) == 0)
		{
			procMethod = (*it).second;
			continue;
		}
		if((*it).first.compare(ACTION) == 0)
		{
			action = (*it).second;
			continue;
		}
	}
	if(action.compare("add") == 0)
	{
		if(procMethod.empty())
		{
			errInfo.append("mineProcess can't be empty");
			WriteLog(mine,ERROR,"mineProcess can't be empty");
			return false;
		}
	}
	if(mineType.empty() || userName.empty() || action.empty())
	{
		errInfo.append("ftpName or mineType or action not valid");
		WriteLog(mine,ERROR,"ftpName or mineType or action invalid");
		return false;
	}

	CVirtualHost *virtualHost;
	bool success = InitEnv(&virtualHost,userName,errInfo,mine);

	if(success && action.compare("add") == 0)
	{
		vector<string> vt_mine;
		vector<string> vt_proc;
		SplitByComas(mineType,vt_mine);
		SplitByComas(procMethod,vt_proc);
		if(vt_mine.size() != vt_proc.size() && vt_proc.size() != 1)
		{
			success = false;
			errInfo.append("the count of the param is not valid");
			WriteLog(mine,ERROR,"the count of the param is not valid");
		}
		if(vt_proc.size() == 1)
		{
			for(int i = 0; i < vt_mine.size(); i++)
			{
				AddMineType(vt_mine[i],vt_proc[0],virtualHost);
			}
		}
		else
		{
			for(int i = 0; i < vt_mine.size(); i++)
			{
				AddMineType(vt_mine[i],vt_proc[i],virtualHost);
			}
		}
	}
	else if(success && action.compare("delete") == 0)
	{
		vector<string> vt_mine;
		SplitByComas(mineType,vt_mine);
		for(int i = 0; i < vt_mine.size();i++)
		{
			DeleteMineType(vt_mine[i],virtualHost);
		}
	}
	if(success)
		success = WriteFile(virtualHost,errInfo,mine);
	if(success)
		WriteParam(mine,vt_param,"success");
	else
		WriteParam(mine,vt_param,"failed");
	CVirtualHost::ReleaseVirtualHost(userName);
	return success;
}


void CAction::DeleteMineType(string &mineType,CVirtualHost *virtualHost)
{
	string directive = "AddType";
	vector<string>::iterator it = virtualHost->GetIterator();
	vector<string> vt_tmp;
	char buf[256];
	while((it = virtualHost->FindGlobalDirective(directive,NULL,0,it)) != virtualHost->GetEndIterator())
	{
		vt_tmp.clear();
		Split(*it,vt_tmp);
		if(vt_tmp.size() == 3 && IsEqualString(vt_tmp[2],mineType))
		{
			string log = "line delete: ";
			log.append(*it);
			sprintf(buf,"%s from %s",log.c_str(),virtualHost->GetFileName().c_str());
			WriteLog(mine,INFO,buf);
			virtualHost->EraseItem(it);
			break;
		}
		it++;
	}
}

void CAction::AddMineType(string &mineType,string &procMethod,CVirtualHost *virtualHost)
{
	char buf[256];
	DeleteMineType(mineType,virtualHost);
	string directive = "AddType";
	vector<string>::iterator it = virtualHost->FindGlobalDirective(directive,NULL,0,virtualHost->GetIterator());
	if(it == virtualHost->GetEndIterator())
	{
		it = virtualHost->GetIterator();
		it++;
	}
	vector<string> param;
	param.push_back(procMethod);
	param.push_back(mineType);
	virtualHost->AddDirective(directive,it,param,4);
	string log = "line add: ";
	log.append(*it);
	sprintf(buf,"%s to %s",log.c_str(),virtualHost->GetFileName().c_str());
	WriteLog(mine,INFO,buf);
}

bool CAction::DirectoryAccess(string &action,string &ip,string &dir,CVirtualHost *virtualHost)
{
	string tmp ="";
	bool allow = false;
	if(action.compare("allow") == 0)
		allow = true;
	else
		allow = false;
	vector<string> vt_dir;
	SplitByComas(dir,vt_dir);
	vector<string> vt_ip;
	SplitByComas(ip,vt_ip);
	if(vt_dir.size() == 0)
	{
		if(vt_ip.size() == 0)
		{
			AddAccess(tmp,tmp,virtualHost,allow);
		}
		else
		{
			for(int i = 0; i < vt_ip.size(); i++)
				AddAccess(vt_ip[i],tmp,virtualHost,allow);
		}
	}
	else if(vt_dir.size() == 1)
	{
		if(vt_ip.size() == 0)
		{
			AddAccess(tmp,vt_dir[0],virtualHost,allow);
		}
		else
		{
			for(int i = 0; i < vt_ip.size(); i++)
				AddAccess(vt_ip[i],vt_dir[0],virtualHost,allow);
		}
	}
	else
	{
		if(vt_ip.size() == 0)
		{
			for(int i = 0; i < vt_dir.size(); i++)
			{
				AddAccess(tmp,vt_dir[i],virtualHost,allow);
			}
		}
		else if(vt_ip.size() == 1)
		{
			for(int i = 0; i < vt_dir.size(); i++)
			{
				AddAccess(vt_ip[0],vt_dir[i],virtualHost,allow);
			}
		}
		else
		{
			if(vt_ip.size() != vt_dir.size())
			{
				return false;
			}
			else
			{
				for(int i = 0; i < vt_dir.size(); i++)
				{
					AddAccess(vt_ip[i],vt_dir[i],virtualHost,allow);
				}
			}
		}
	}
	return true;
}

bool CAction::ProcDirectoryAccess(vector<pair<string,string> > &vt_param,string &errInfo)
{
	WriteParam(directoryAccess,vt_param,"");

	if(vt_param.size() < 5)
	{
		errInfo.append("too less param.");
		return false;
	}

	string action = "";
	string ip = "";
	string dir = "";
	string username = "";

	vector<pair<string,string> >::iterator it = vt_param.begin();
	it++;
	it++;
	for(;it != vt_param.end(); it++)
	{
		if((*it).first.compare(ACTION) == 0)
		{
			action = (*it).second;
			continue;
		}
		if((*it).first.compare(ADDRESS) == 0)
		{
			ip = (*it).second;
			continue;
		}
		if((*it).first.compare(DIRECTORY) == 0)
		{
			dir = (*it).second;
			continue;
		}
		if((*it).first.compare(USERNAME) == 0)
		{
			username = (*it).second;
			continue;
		}
	}
	
	if(username.empty() || action.empty())
	{
		errInfo.append("ftpName or action not valid");
		WriteLog(directoryAccess,ERROR,"ftpName or action invalid");
		return false;
	}

	CVirtualHost *virtualHost;
	bool success = InitEnv(&virtualHost,username,errInfo,directoryAccess);
	
	if(success)
	{
		success = DirectoryAccess(action,ip,dir,virtualHost);
		if(!success)
			errInfo.append("the directory or the ip  is not valid.");
	}
	if(success)
		success = WriteFile(virtualHost,errInfo,mine);
	if(success)
		WriteParam(mine,vt_param,"success");
	else
		WriteParam(mine,vt_param,"failed");
	CVirtualHost::ReleaseVirtualHost(username);
	return success;
}

void CAction::AddAccess(string &ip,string &dir,CVirtualHost *virtualHost,bool allow)
{
	vector<string> vt_param;
	if(ip.empty())
	{
		AddAccessIpEmpty(dir,virtualHost,allow);
	}
	else
	{
		AddAccessIpNotEmpty(dir,virtualHost,ip,allow);
	}
}

void CAction::AddAccessIpNotEmpty(string &dir,CVirtualHost *virtualHost,string &ip,bool allow)
{
	if(dir.empty())
	{
		AddAccessDirEmptyIpNotEmpty(virtualHost,ip,allow);
	}
	else
	{
		AddAccessDirNotEmptyIpNotEmpty(virtualHost,ip,dir,allow);
	}
}

void CAction::AddAccessIpEmpty(string &dir,CVirtualHost *virtualHost,bool allow)
{
	if(dir.empty())
	{
		AddAccessDirEmptyIpEmpty(virtualHost,allow);
	}
	else
	{
		AddAccessDirNotEmptyIpEmpty(virtualHost,dir,allow);	
	}
}

void CAction::DeleteNodeDirective(vector<string>::iterator it,string &directive,vector<string> vt_param,CVirtualHost *virtualHost)
{
	vector<string>::iterator it_tmp;
	while(1)
	{
		it_tmp = virtualHost->FindNodeDirective(it,directive,vt_param);
		
		if(it_tmp != virtualHost->GetEndIterator())
			virtualHost->EraseItem(it_tmp);
		else
			break;
	}		
}

void CAction::AddAccessDirEmptyIpEmpty(CVirtualHost *virtualHost,bool allow)
{
	vector<string> vt_param;
	string nodeName = "Directory";
	vector<string>::iterator it = virtualHost->GetIterator();
	while(1)
	{
		vt_param.clear();
		it = virtualHost->FindNode(nodeName,vt_param,it);
		if(it == virtualHost->GetEndIterator())
			break;
		string directive = ORDER;
		DeleteNodeDirective(it,directive,vt_param,virtualHost);
		directive = ALLOW;
		DeleteNodeDirective(it,directive,vt_param,virtualHost);
		directive = DENY;
		DeleteNodeDirective(it,directive,vt_param,virtualHost);
		it++;
		directive = ORDER;
		if(allow)
			vt_param.push_back("deny,allow");
		else
			vt_param.push_back("allow,deny");
		virtualHost->AddDirective(directive,it,vt_param,8);
		it++;
		vt_param.clear();
		vt_param.push_back("from");
		vt_param.push_back("all");
		if(allow)	
			directive = DENY;
		else
			directive = ALLOW;
		virtualHost->AddDirective(directive,it,vt_param,8);
	}
}

void CAction::AddAccessDirNotEmptyIpEmpty(CVirtualHost *virtualHost,string &dir,bool allow)
{
	vector<string> vt_param;
	string nodeName = "Directory";
	vt_param.push_back(dir);
	vector<string>::iterator it = virtualHost->FindNode(nodeName,vt_param,virtualHost->GetIterator());
	string directive = ORDER;
	if(it == virtualHost->GetEndIterator())
	{
		it = virtualHost->GetIterator(1);
		string t = "\"";
		t.append(dir);
		t.append("\"");
		vt_param.push_back(t);
		it = virtualHost->AddNode(t,it,vt_param);
		vt_param.clear();
		if(allow)
			vt_param.push_back("deny,allow");
		else
			vt_param.push_back("allow,deny");
		virtualHost->AddDirective(directive,it,vt_param,8);
		it++;
		vt_param.clear();
		vt_param.push_back("from");
		vt_param.push_back("all");
		if(allow)
			directive = DENY;
		else
			directive = ALLOW;
		virtualHost->AddDirective(directive,it,vt_param,8);
	}
	else
	{
		vector<string>::iterator it_tmp;
		vt_param.clear();
		it_tmp = virtualHost->FindNodeDirective(it,directive,vt_param);
		//如果找到了，则删除相关信息
		if(it_tmp != virtualHost->GetEndIterator())
		{
			virtualHost->EraseItem(it_tmp);

			directive = ALLOW;
			DeleteNodeDirective(it,directive,vt_param,virtualHost);	
			directive = DENY;
			DeleteNodeDirective(it,directive,vt_param,virtualHost);
		}
		it++;
		vt_param.clear();
		if(allow)
			vt_param.push_back("deny,allow");
		else
			vt_param.push_back("allow,deny");
		virtualHost->AddDirective(directive,it,vt_param,8);
		it++;
		vt_param.clear();
		vt_param.push_back("from");
		vt_param.push_back("all");
		if(allow)
			directive = DENY;
		else
			directive = ALLOW;
		virtualHost->AddDirective(directive,it,vt_param,8);
	}
}

void CAction::AddAccessDirEmptyIpNotEmpty(CVirtualHost *virtualHost,string &ip,bool allow)
{
	vector<string> vt_param;
	string nodeName = "Directory";
	vector<string>::iterator it = virtualHost->GetIterator();
	while(1)
	{
		vt_param.clear();
		it = virtualHost->FindNode(nodeName,vt_param,it);
		if(it == virtualHost->GetEndIterator())
			break;
		string directive = ORDER;
		vector<string>::iterator it_tmp = virtualHost->FindNodeDirective(it,directive,vt_param);
		if(it_tmp == virtualHost->GetEndIterator())
		{
			directive = ALLOW;
			DeleteNodeDirective(it,directive,vt_param,virtualHost);	
			directive = DENY;
			DeleteNodeDirective(it,directive,vt_param,virtualHost);	
			it++;
			vt_param.clear();
			directive = ORDER;
			if(allow)
				vt_param.push_back("deny,allow");
			else
				vt_param.push_back("allow,deny");
			virtualHost->AddDirective(directive,it,vt_param,8);
			it++;
			if(allow)
				directive = DENY;
			else
				directive = ALLOW;
			vt_param.clear();
			vt_param.push_back("from");
			vt_param.push_back("all");
			virtualHost->AddDirective(directive,it,vt_param,8);
			it++;

			vt_param.clear();
			vt_param.push_back("from");
			vt_param.push_back(ip);
			if(allow)
				directive = ALLOW;
			else
				directive = DENY;
			virtualHost->AddDirective(directive,it,vt_param,8);
			continue;
		}
		vector<string> vt_split;
		Split((*it_tmp),vt_split);
		if(strncmp(vt_split[1].c_str(),DENY,strlen(DENY)) == 0)
		{
			directive = ALLOW;
			vt_param.clear();
			vt_param.push_back("from");
			vt_param.push_back(ip);
			vector<string>::iterator i = virtualHost->FindNodeDirective(it,directive,vt_param);
			if(i == virtualHost->GetEndIterator() && allow)
			{
				it_tmp++;
				virtualHost->AddDirective(directive,it_tmp,vt_param,8);
			}
			if(!allow && i != virtualHost->GetEndIterator())
			{
				virtualHost->EraseItem(i);
			}
		}
		else
		{
			directive = DENY;
			vt_param.clear();
			vt_param.push_back("from");
			vt_param.push_back(ip);
			vector<string>::iterator i = virtualHost->FindNodeDirective(it,directive,vt_param);
			if(i != virtualHost->GetEndIterator() && allow)
				virtualHost->EraseItem(i);
			if(i == virtualHost->GetEndIterator() && !allow)
			{
				it_tmp++;
				virtualHost->AddDirective(directive,it_tmp,vt_param,8);
			}
		}
		it++;
	}
}

void CAction::AddAccessDirNotEmptyIpNotEmpty(CVirtualHost *virtualHost,string &ip,string &dir,bool allow)
{
	vector<string> vt_param;
	string nodeName = "Directory";
	vt_param.push_back(dir);
	vector<string>::iterator it = virtualHost->FindNode(nodeName,vt_param,virtualHost->GetIterator());
	string directive = ORDER;
	if(it == virtualHost->GetEndIterator())
	{
		it = virtualHost->GetIterator(1);
		string t = "\"";
		t.append(dir);
		t.append("\"");
		vt_param.push_back(t);
		it = virtualHost->AddNode(t,it,vt_param);
		vt_param.clear();
		if(allow)
			vt_param.push_back("deny,allow");
		else
			vt_param.push_back("allow,deny");
		virtualHost->AddDirective(directive,it,vt_param,8);
		it++;
		if(allow)
			directive = DENY;
		else
			directive = ALLOW;
		vt_param.clear();
		vt_param.push_back("from");
		vt_param.push_back("all");
		virtualHost->AddDirective(directive,it,vt_param,8);
		it++;

		vt_param.clear();
		vt_param.push_back("from");
		vt_param.push_back(ip);
		if(allow)
			directive = ALLOW;
		else
			directive = DENY;
		virtualHost->AddDirective(directive,it,vt_param,8);
	}
	else
	{
		vt_param.clear();
		vector<string>::iterator it_tmp = virtualHost->FindNodeDirective(it,directive,vt_param);
		if(it_tmp == virtualHost->GetEndIterator())
		{
			directive = ALLOW;
			DeleteNodeDirective(it,directive,vt_param,virtualHost);	
			directive = DENY;
			DeleteNodeDirective(it,directive,vt_param,virtualHost);	
			it++;
			vt_param.clear();
			directive = ORDER;
			if(allow)
				vt_param.push_back("deny,allow");
			else
				vt_param.push_back("allow,deny");
			virtualHost->AddDirective(directive,it,vt_param,8);
			it++;
			if(allow)
				directive = DENY;
			else
				directive = ALLOW;
			vt_param.clear();
			vt_param.push_back("from");
			vt_param.push_back("all");
			virtualHost->AddDirective(directive,it,vt_param,8);
			it++;

			vt_param.clear();
			vt_param.push_back("from");
			vt_param.push_back(ip);
			if(allow)
				directive = ALLOW;
			else
				directive = DENY;
			virtualHost->AddDirective(directive,it,vt_param,8);
		}
		else
		{
			vector<string> vt_split;
			Split(*it_tmp,vt_split);
			if(strncmp(vt_split[1].c_str(),DENY,strlen(DENY)) == 0)
			{
				directive = ALLOW;
				vt_param.clear();
				vt_param.push_back("from");
				vt_param.push_back(ip);
				vector<string>::iterator i = virtualHost->FindNodeDirective(it,directive,vt_param);
				if(i == virtualHost->GetEndIterator() && allow)
				{
					it_tmp++;
					virtualHost->AddDirective(directive,it_tmp,vt_param,8);
				}
				if(i != virtualHost->GetEndIterator() && !allow)
				{
					virtualHost->EraseItem(i);
				}
			}
			else
			{
				if(allow)
					directive = DENY;
				else
					directive = ALLOW;
				vt_param.clear();
				vt_param.push_back("from");
				vt_param.push_back(ip);
				vector<string>::iterator i = virtualHost->FindNodeDirective(it,directive,vt_param);
				if(i != virtualHost->GetEndIterator() && allow)
					virtualHost->EraseItem(i);
				if(i == virtualHost->GetEndIterator() && !allow)
				{
					it_tmp++;
					virtualHost->AddDirective(directive,it_tmp,vt_param,8);
				}
			}
		}
	}
}
