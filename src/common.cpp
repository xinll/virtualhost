/*************************************************************************
    > File Name: common.cpp
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年09月27日 星期五 13时23分04秒
 ************************************************************************/

#include "common.h"
#include "tools.h"
#include <stdio.h>
#include <string.h>

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
	bool node  = false;
	for(; it != vt_conf.end(); it++)
	{
		const char* tmp = (*it).c_str();
		while((*tmp) == 32 || (*tmp) == 9) //获取第一个非空格和tab的字符
			tmp++;
		if(*tmp == '#')
			continue;
		if(*tmp == '<')
		{
			if(strncasecmp("virtualhost",tmp+1,strlen("virtualhost")) == 0)
				continue;
			else if(*(tmp+1) == '/')
			{
				node = false;
			}
			else
				node = true;
			continue;
		}
		exist = true;
		vt_tmp.clear();
		Split((*it),vt_tmp);
		if(vt_tmp.size() > 0 && IsEqualString(vt_tmp[0],directive) && !node)
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

vector<string>::iterator CVirtualHost::EraseItem(vector<string>::iterator it)
{
	return vt_conf.erase(it);
}
bool CVirtualHost::AddNode(string &node,vector<string>::iterator &it,string nodeParam[],int n)
{
	;
}

void CVirtualHost::AddDirective(string &directive,vector<string>::iterator &it,string nodeParam[],int n)
{
	string oneRecord = directive;
	for(int i = 0; i < n; i++)
	{
		oneRecord.append(" ");
		oneRecord.append(nodeParam[i]);
	}
	oneRecord.append(NEWLINE);
	vt_conf.insert(it,oneRecord);
}
