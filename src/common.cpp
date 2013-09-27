/*************************************************************************
    > File Name: common.cpp
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年09月27日 星期五 13时23分04秒
 ************************************************************************/

#include "common.h"
#include "tools.h"
#include <stdio.h>

CVirtualHost::CVirtualHost(string &ftpName)
{
	this->ftpName = ftpName;
	lastModified  = time(NULL);
	errorInfo     = "";
}

int CVirtualHost::GetSecondsDiff()
{
	return time(NULL) - lastModified;
}

bool CVirtualHost::LoadFile()
{
	string path = MakeConfPath(ftpName);
	if(!ReadFile(&vt_conf,path.c_str()))
	{
		char buf[256];
		sprintf(buf,"读取配置文件%s.conf失败",ftpName.c_str());
		errorInfo = buf;
		return false;
	}
	return true;
}

bool CVirtualHost::SaveFile()
{
	string path = MakeConfPath(ftpName);
	if(!WriteFile(&vt_conf,path.c_str()))
	{
		char buf[256];
		sprintf(buf,"写入配置文件%s.conf失败",ftpName.c_str());
		errorInfo = buf;
		return false;
	}
	return true;
}

vector<string>::iterator CVirtualHost::GetIterator(int offset)
{
	if(offset > vt_conf.size() || offset < 0)
	{
		return vt_conf.begin();
	}
	vector<string>::iterator it = vt_conf.begin();
	it += offset;
	return it;
}

vector<string>::iterator CVirtualHost::GetEndIterator()
{
	vector<string>::iterator it = vt_conf.end();
	return it;
}

int CVirtualHost::FindNode(string &node,string &nodeParam)
{
	;
}

vector<string>::iterator& FindGlobalDirective(string &directive)
{
	;
}

int CVirtualHost::FindNodeDirective()
{
	;
}

bool CVirtualHost::AddNode(string &node,vector<string>::iterator &it,string &nodeParam)
{
	;
}

bool AddDirective(string &directive,vector<string>::iterator &it,string nodeParam[])
{
	;
}
