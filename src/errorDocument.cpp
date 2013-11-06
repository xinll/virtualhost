/*************************************************************************
    > File Name: errorDocument.cpp
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年11月04日 星期一 09时57分13秒
 ************************************************************************/

#include "tools.h"
#include "log.h"
#include "string.h"
#include "defines.h"
#include "errorDocument.h"
#include "common.h"

static char errorDocument[] = "404Log";
bool ProcErrorDocument(vector<pair<string,string> > &vt_param,string &errInfo)
{
	WriteParam(errorDocument,vt_param,"");

	if(!CheckParam(vt_param,5,errInfo))
		return false;

	string errorNum = GetValue(ERRORNMSTR,vt_param);
	string errorPage = GetValue(ERRORPAGE,vt_param);
	string userName = GetValue(USERNAME,vt_param);

	if(!ValidateParamEmpty(errorNum.c_str()) || !ValidateParamEmpty(userName.c_str()))
	{
		WriteLog(errorDocument,ERROR,"errorNum or userName invalid");
		errInfo.append("ftpName or errorNum invalid.");
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
		success = WriteVirtualHost(virtualHost,errInfo,errorDocument);
	}
	if(success)
		WriteParam(errorDocument,vt_param,"success");
	else
		WriteParam(errorDocument,vt_param,"failed");
	CVirtualHost::ReleaseVirtualHost(userName);
	return success;
}
