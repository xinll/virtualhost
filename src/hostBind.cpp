/*************************************************************************
    > File Name: hostBind.cpp
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年11月06日 星期三 17时06分03秒
 ************************************************************************/

#include "directoryIndex.h"
#include "defines.h"
#include "tools.h"
#include "log.h"
#include "common.h"

static char log[] = "hostBind";

bool ProcBind(vector<pair<string,string> > &vt_param,string &error)
{
	WriteParam(log,vt_param,"");

	if(!CheckParam(vt_param,4,error))
		return false;

	string userName = GetValue(USERNAME,vt_param);
	string bind = GetValue(HOSTBIND,vt_param);

	if(!ValidateParamEmpty(userName.c_str()) || !ValidateParamEmpty(bind.c_str()))
	{
		char info[] = "ftpName or index invalid";
		WriteLog(log,ERROR,info);
		error.append(info);
		return false;
	}

	
	CVirtualHost *virtualHost;
	bool success = InitEnv(&virtualHost,userName,error,log);

	if(success)
	{
		string directive = SERVERALIAS;
		vector<string>::iterator it = virtualHost->FindGlobalDirective(directive,NULL,0,virtualHost->GetIterator());

		if(it != virtualHost->GetEndIterator())
		{
			it = virtualHost->EraseItem(it);
		}
		else
		{
			it = virtualHost->GetIterator(1);
		}
		
		vector<string> vt;
		SplitByComas(bind,vt,',');
		
		int size = vt.size();
		string param;
		for(int i = 0;i < size; i++)
		{
			param.append(vt[i]);
			if(i != size - 1)
				param.append(" ");
		}

		vt.clear();
		vt.push_back(param);
		virtualHost->AddDirective(directive,it,vt,4);
		success = WriteVirtualHost(virtualHost,error,log);
	}

	success ? WriteParam(log,vt_param,"success") : WriteParam(log,vt_param,"failed");

	CVirtualHost::ReleaseVirtualHost(userName);
	return success;
}
