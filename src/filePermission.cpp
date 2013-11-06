/*************************************************************************
    > File Name: filePermission.cpp
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年11月04日 星期一 10时38分22秒
 ************************************************************************/

#include "log.h"
#include "tools.h"
#include "defines.h"
#include "filePermission.h"
#include <stdlib.h>

static char filePermission[] = "filePermissionLog";

bool ProcFilePermission(vector<pair<string,string> > &vt_param,string &errInfo)
{	
	WriteParam(filePermission,vt_param,"");

	if(!CheckParam(vt_param,4,errInfo))
		return false;

	int permission = -1;
	string directory = GetValue(DIRECTORY,vt_param);
	string userName = GetValue(USERNAME,vt_param);
	string strPermission = GetValue(PERMISSION,vt_param);
	permission = atoi(strPermission.c_str());

	//匹配目录下所有文件
	string file = "\"\\w*\"";
	if(!ValidateParamEmpty(userName.c_str()))
	{
		WriteLog(filePermission,ERROR,"ftpName invalid");
		errInfo.append("ftpName invalid.");
		return false;
	}
	if(permission < 0 || permission > 1)
	{
		errInfo.append("permission not valid");
		WriteLog(filePermission,ERROR,"permission invalid");
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
				it_node = virtualHost->AddDirective(directive,it_node,vt_tmpParam,8);
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
				it_node = virtualHost->AddDirective(directive,it_node,vt_tmpParam,8);
				it_node++;
				vt_tmpParam.clear();
				vt_tmpParam.push_back("from");
				vt_tmpParam.push_back("all");
				directive = DENY;
				virtualHost->AddDirective(directive,it_node,vt_tmpParam,8);
				break;
			}
		}
		
		success = WriteVirtualHost(virtualHost,errInfo,filePermission);
	}
	if(success)
		WriteParam(filePermission,vt_param,"success");
	else
		WriteParam(filePermission,vt_param,"failed");
	CVirtualHost::ReleaseVirtualHost(userName);
	return success;
}
