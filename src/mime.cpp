/*************************************************************************
    > File Name: mime.cpp
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年11月04日 星期一 11时37分20秒
 ************************************************************************/

#include "log.h"
#include "common.h"
#include "tools.h"
#include "defines.h"
#include "mime.h"

static char mine[] = "mineLog";
void DeleteMineType(string &mineType,CVirtualHost *virtualHost)
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

void AddMineType(string &mineType,string &procMethod,CVirtualHost *virtualHost)
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
	it = virtualHost->AddDirective(directive,it,param,4);
	string log = "line add: ";
	log.append(*it);
	sprintf(buf,"%s to %s",log.c_str(),virtualHost->GetFileName().c_str());
	WriteLog(mine,INFO,buf);
}

bool ProcMineType(vector<pair<string,string> > &vt_param,string &errInfo)
{
	WriteParam(mine,vt_param,"");

	string userName = GetValue(USERNAME,vt_param);
	string action = GetValue(ACTION,vt_param);
	string mineType = GetValue(MINETYPE,vt_param);
	string procMethod = GetValue(MINEPROCESS,vt_param);

	if(action.compare("add") == 0)
	{
		if(!ValidateParamEmpty(action.c_str()))
		{
			errInfo.append("mineProcess can't be empty.");
			WriteParam(mine,vt_param,"failed. process can't be empty");
			return false;
		}
	}

	if(!ValidateParamEmpty(mineType.c_str()) || !ValidateParamEmpty(userName.c_str()) || !ValidateParamEmpty(action.c_str()))
	{
		errInfo.append("ftpName or mineType or action not valid.");
		WriteParam(mine,vt_param,"ftpName or mineType or action invalid");
		return false;
	}

	CVirtualHost *virtualHost;
	bool success = InitEnv(&virtualHost,userName,mine);
	if(!success)
	{
		errInfo.append("failed to process your request.");
		CVirtualHost::ReleaseVirtualHost(userName);
		return false;
	}

	if(success && action.compare("add") == 0)
	{
		AddMineType(mineType,procMethod,virtualHost);
	}
	else if(success && action.compare("delete") == 0)
	{
		DeleteMineType(mineType,virtualHost);
	}
	if(success)
		success = virtualHost->SaveFile();
	if(success)
		WriteParam(mine,vt_param,"success");
	else
	{
		WriteParam(mine,vt_param,"failed. write the config file failed");
		WriteParam(mine,vt_param,"failed");
	}
	CVirtualHost::ReleaseVirtualHost(userName);
	return success;
}

