/*************************************************************************
    > File Name: redirect.cpp
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年11月04日 星期一 11时26分33秒
 ************************************************************************/
#include "common.h"
#include "tools.h"
#include "log.h"
#include "stdio.h"
#include "defines.h"
#include "redirect.h"

static char redirect[] = "redirectLog";
void AddRedirect(string redirectFrom,string redirectTo,CVirtualHost *virtualHost)
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
		it_insert++;

		directive = "RewriteCond";
		vt_tmpParam.push_back("%{HTTP_POST}");
		string tmp = "^";
		tmp.append(redirectFrom);
		tmp.append("$");
		vt_tmpParam.push_back(tmp);
		vt_tmpParam.push_back("[NC]");
		it_insert = virtualHost->AddDirective(directive,it_insert,vt_tmpParam,4);
		string log = "line add: ";
		log.append(*(it_insert));
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
	it = virtualHost->AddDirective(directive,it,vt_tmpParam,4);
	string log = "line add: ";
	log.append(*it);
	sprintf(buf,"%s from %s",log.c_str(),virtualHost->GetFileName().c_str());
	WriteLog(redirect,INFO,buf);
}

void DeleteRedirect(string redirectFrom,CVirtualHost *virtualHost)

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

bool ProcRedirect(vector<pair<string,string> > &vt_param,string &errInfo)
{
	WriteParam(redirect,vt_param,"");

	if(!CheckParam(vt_param,4,errInfo))
		return false;

	string userName = GetValue(USERNAME,vt_param);
	string url = GetValue(URL,vt_param);

	if(!ValidateParamEmpty(userName.c_str()) || !ValidateParamEmpty(url.c_str()))
	{
		errInfo.append("ftpName or url not valid.");
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
			to = "";
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
		success = WriteVirtualHost(virtualHost,errInfo,redirect);

	if(success)
		WriteParam(redirect,vt_param,"success");
	else
		WriteParam(redirect,vt_param,"failed");
	CVirtualHost::ReleaseVirtualHost(userName);
	return success;
}
