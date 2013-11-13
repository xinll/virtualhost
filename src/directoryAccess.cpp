/*************************************************************************
    > File Name: action.cpp
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年09月29日 星期日 13时27分09秒
 ************************************************************************/

#include "common.h"
#include "defines.h"
#include "tools.h"
#include "directoryAccess.h"
#include <string.h>
#include "log.h"
#include <dirent.h>
#include <sys/types.h>

static char directoryAccess[] = "directoryAccessLog";

void DeleteNodeDirective(vector<string>::iterator it,string &directive,vector<string> vt_param,CVirtualHost *virtualHost)
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

bool AddAccessDirNotEmptyIpEmpty(CVirtualHost *virtualHost,string &dir,bool allow)
{
	vector<string> vt_param;
	string nodeName = "Directory";
	vt_param.push_back(dir);
	vector<string>::iterator it = virtualHost->FindNode(nodeName,vt_param,virtualHost->GetIterator());
	string directive = ORDER;
	if(it == virtualHost->GetEndIterator())
	{
		vt_param.clear();
		//it = virtualHost->GetIterator(1);
		it = virtualHost->FindNode(nodeName,vt_param,virtualHost->GetIterator());

		string t = "\"";
		t.append(dir);
		t.append("\"");
		vt_param.push_back(t);
		it = virtualHost->AddNode(nodeName,it,vt_param);
		vt_param.clear();
		if(allow)
			//vt_param.push_back("deny,allow");
			vt_param.push_back("allow,deny");
		else
			//vt_param.push_back("allow,deny");
			vt_param.push_back("deny,allow");
		it = virtualHost->AddDirective(directive,it,vt_param,8);
		it++;
		vt_param.clear();
		vt_param.push_back("from");
		vt_param.push_back("all");
		//directive = allow ? DENY : ALLOW;
		directive = allow ? ALLOW : DENY;
		it = virtualHost->AddDirective(directive,it,vt_param,8);
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
			//vt_param.push_back("deny,allow");
			vt_param.push_back("allow,deny");
		else
			//vt_param.push_back("allow,deny");
			vt_param.push_back("deny,allow");
		directive = ORDER;
		it = virtualHost->AddDirective(directive,it,vt_param,8);
		it++;
		vt_param.clear();
		vt_param.push_back("from");
		vt_param.push_back("all");
		//directive = allow ? DENY : ALLOW;
		directive = allow ? ALLOW : DENY;
		virtualHost->AddDirective(directive,it,vt_param,8);
	}
	return true;
}

bool AddAccessDirNotEmptyIpNotEmpty(CVirtualHost *virtualHost,string &ip,string &dir,bool allow)
{
	vector<string> vt_param;
	string nodeName = "Directory";
	vt_param.push_back(dir);
	vector<string>::iterator it = virtualHost->FindNode(nodeName,vt_param,virtualHost->GetIterator());
	string directive = ORDER;
	if(it == virtualHost->GetEndIterator())
	{
		vt_param.clear();
	//	it = virtualHost->GetIterator(1);
		it = virtualHost->FindNode(nodeName,vt_param,virtualHost->GetIterator());

		string t = "\"";
		t.append(dir);
		t.append("\"");
		vt_param.push_back(t);
		it = virtualHost->AddNode(nodeName,it,vt_param);
		vt_param.clear();
		if(allow)
			//vt_param.push_back("deny,allow");
			vt_param.push_back("allow,deny");
		else
			//vt_param.push_back("allow,deny");
			vt_param.push_back("deny,allow");
		it = virtualHost->AddDirective(directive,it,vt_param,8);
		it++;
		if(allow)
			directive = ALLOW; //DENY;
		else
			directive = DENY; //ALLOW;
		vt_param.clear();
		vt_param.push_back("from");
		vt_param.push_back("all");
		it = virtualHost->AddDirective(directive,it,vt_param,8);
		it++;

		vt_param.clear();
		vt_param.push_back("from");
		vt_param.push_back(ip);
		//directive = allow ? ALLOW : DENY;
		directive = allow ? DENY : ALLOW;
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
				//vt_param.push_back("deny,allow");
				vt_param.push_back("allow,deny");
			else
				//vt_param.push_back("allow,deny");
				vt_param.push_back("deny,allow");
			it = virtualHost->AddDirective(directive,it,vt_param,8);
			it++;
			if(allow)
				directive = ALLOW; //DENY;
			else
				directive = DENY; //ALLOW;
			vt_param.clear();
			vt_param.push_back("from");
			vt_param.push_back("all");
			it = virtualHost->AddDirective(directive,it,vt_param,8);
			it++;

			vt_param.clear();
			vt_param.push_back("from");
			vt_param.push_back(ip);
			//directive = allow ? ALLOW : DENY;
			directive = allow ? DENY : ALLOW;
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
				if(allow && i != virtualHost->GetEndIterator())
				{
					virtualHost->EraseItem(i);
				}
				if(!allow && i == virtualHost->GetEndIterator())
				{
					it_tmp++;
					virtualHost->AddDirective(directive,it_tmp,vt_param,8);
				}
			}
			else
			{
				directive = DENY;
				vt_param.clear();
				vt_param.push_back("from");
				vt_param.push_back(ip);
				vector<string>::iterator i = virtualHost->FindNodeDirective(it,directive,vt_param);
			    if(i != virtualHost->GetEndIterator() && !allow)
					virtualHost->EraseItem(i);
			    if(i == virtualHost->GetEndIterator() && allow)
				{
					it_tmp++;
					virtualHost->AddDirective(directive,it_tmp,vt_param,8);
				}
			}
		}
	}
	return true;
}

bool DirectoryAccess(string &action,string &ip,string &dir,CVirtualHost *virtualHost)
{
	string tmp ="";
	bool allow = false;
	if(action.compare("allow") == 0)
		allow = true;
	else
		allow = false;
	vector<string> vt_ip;
	SplitByComas(ip,vt_ip);
	//限制网站的，只要限制根目录就OK
	string path = GetEnvVar("USER_ROOT");
	if(path.empty())
		path = USER_ROOT;
	AddSlash(path);
	path.append(virtualHost->GetFileName());
	AddSlash(path);

	path.append("home");
/*	if(dir.empty())
	{
		path.append("/wwwroot");
	}
	else
	{
		path = MakePath(path,dir);
	}*/

	MakePath(path,"/wwwroot");
	MakePath(path,dir);
	DIR *d = opendir(path.c_str());
	if(d == NULL)
	{
		WriteLog(directoryAccess,ERROR,"the Directory does not exist");
		return false;
	}
	closedir(d);

	if(dir.empty())
	{
		bool success = false;
		int size = vt_ip.size();
		if(size == 0)
			return AddAccessDirNotEmptyIpEmpty(virtualHost,path,allow);
		vector<string> vt_tmp;
		for(int i = 0; i < size; i++)
		{
			vt_tmp.clear();
			SplitByComas(vt_ip[i],vt_tmp,':');
			string tmp_ip = vt_tmp[0];
			if(vt_tmp.size() > 1)
			{
				tmp_ip.append("/");
				tmp_ip.append(vt_tmp[1]);
			}
			success = AddAccessDirNotEmptyIpNotEmpty(virtualHost,tmp_ip,path,allow);
		}
		return success;
	}
	else
	{
		bool success = false;
		int size = vt_ip.size();
		if(size == 0)
			return AddAccessDirNotEmptyIpEmpty(virtualHost,path,allow);
		vector<string> vt_tmp;
		int mask = 0;
		string tmp_ip;
		//处理mask
	//	vector<string> vt_mask;
		for(int i = 0; i < size; i++)
		{
			//处理mask
		/*	vt_mask.clear();
			SplitByComas(vt_ip[i],vt_mask,':');
			tmp_ip = vt_mask[0];*/
			
			vt_tmp.clear();
			SplitByComas(vt_ip[i],vt_tmp,':');
			tmp_ip = vt_tmp[0];
			if(vt_tmp.size() > 1)
			{
				tmp_ip.append("/");
				tmp_ip.append(vt_tmp[1]);
			}
		/*	mask = 32;
			vt_tmp.clear();
			SplitByComas(vt_ip[i],vt_tmp,'.');
			SplitByComas(vt_mask[0],vt_tmp,'.');
			for(int j = vt_tmp.size() - 1; j >= 0; j--)
			{
				if(vt_tmp[j].compare("0") == 0)
				{
					mask -= 8;
				}
				else
					break;
			}
			if(mask != 0)
			{
				char strMask[128];
				sprintf(strMask,"/%d",mask);
				tmp_ip.append(strMask);
			}*/
			success = AddAccessDirNotEmptyIpNotEmpty(virtualHost,tmp_ip,path,allow);
		}
		return success;
	}
	return true;
}

bool ProcDirectoryAccess(vector<pair<string,string> > &vt_param,string &errInfo)
{
	WriteParam(directoryAccess,vt_param,"");

	if(!CheckParam(vt_param,5,errInfo))
		return false;

	string action = GetValue(ACTION,vt_param);
	string ip = GetValue(ADDRESS,vt_param);
	string dir = GetValue(DIRECTORY,vt_param);
	string username = GetValue(USERNAME,vt_param);

	if(!ValidateParamEmpty(username.c_str()) || !ValidateParamEmpty(action.c_str()))
	{
		char info[] = "ftpName or action invalid.";
		errInfo.append(info);
		WriteLog(directoryAccess,ERROR,info);
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
		success = WriteVirtualHost(virtualHost,errInfo,directoryAccess);
	if(success)
		WriteParam(directoryAccess,vt_param,"success");
	else
		WriteParam(directoryAccess,vt_param,"failed");
	CVirtualHost::ReleaseVirtualHost(username);
	return success;
}

