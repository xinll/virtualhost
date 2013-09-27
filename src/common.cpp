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

int CVirtualHost::FindNode(string &node,string nodeParam[],int n)
{
	;
}

vector<string>::iterator CVirtualHost::FindGlobalDirective(string &directive,string param[],int n)
{
	vector<string>::iterator it = vt_conf.begin();
	vector<string> vt_tmp;
	bool exist = true;
	for(; it < vt_conf.end(); it++)
	{
		exist = true;
		vt_tmp.clear();
		Split((*it),vt_tmp);
		if(vt_tmp.size() > 0 && IsEqualString(vt_tmp[0],directive))
		{
			if(vt_tmp.size() < n)
				continue;
			for(int i = 0; i < n; i++)
			{
				if(!StrInVt(param[i],vt_tmp))
				{
					exist = false;
					break;
				}
			}
			if(!exist)
				continue;
			else
				break;
		}
	}
	return it;
}

int CVirtualHost::FindNodeDirective()
{
	;
}

bool CVirtualHost::AddNode(string &node,vector<string>::iterator &it,string nodeParam[],int n)
{
	;
}

bool AddDirective(string &directive,vector<string>::iterator &it,string nodeParam[],int n)
{
	;
}
