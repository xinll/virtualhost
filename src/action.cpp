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
#include "changePermission.h"
bool CAction::ProcErrorDocument(vector<pair<string,string> > &vt_param,string &errInfo)
{
	if(vt_param.size() < 5)
	{
		errInfo.append("参数太少");
		errInfo.append(SPLIT);
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
	if(errorNum.empty() || errorPage.empty() || userName.empty())
	{
		errInfo.append("errorNum或errorPage或ftpName不合法");
		errInfo.append(SPLIT);
		return false;
	}

	CVirtualHost* virtualHost = CVirtualHost::GetVirtualHost(userName);
	if(virtualHost == NULL)
	{
		errInfo.append("文件正在使用中:");
		errInfo.append(userName);
		errInfo.append(SPLIT);
		return false;
	}

	bool success = true;
	if(!BakConf(userName))
	{
		errInfo.append("备份配置文件失败:");
		errInfo.append(userName);
		errInfo.append(SPLIT);
		success = false;
	}

	if(success && !virtualHost->LoadFile())
	{
		string err = virtualHost->GetLastErrorStr();
		errInfo.append(err);
		errInfo.append(SPLIT);
		success = false;
	}
	if(success)
	{
		string par[2] = {errorNum,errorPage};
		string directive = ERRORHEAD;
		vector<string>::iterator it_directive = virtualHost->FindGlobalDirective(directive,par,1,virtualHost->GetIterator());
		if(it_directive != virtualHost->GetEndIterator())
		{
			it_directive = virtualHost->EraseItem(it_directive);
		}
		else
		{
			it_directive = virtualHost->GetIterator(1);
		}
		virtualHost->AddDirective(directive,it_directive,par,2);
		if(!virtualHost->SaveFile())
		{
			success = false;
			string err = virtualHost->GetLastErrorStr();
			errInfo.append(err);
			errInfo.append(SPLIT);
			if(RestoreConf(userName))
			{
				errInfo.append("恢复配置文件失败:");
				errInfo.append(userName);
				errInfo.append(SPLIT);
			}
		}
	}
	CVirtualHost::ReleaseVirtualHost(userName);
	return success;
}

bool CAction::ProcFilePermission(vector<pair<string,string> > &vt_param,string &errInfo)
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
