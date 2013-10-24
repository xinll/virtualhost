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
#include "defines.h"
#include <syslog.h>

extern pthread_mutex_t mutex;
vector<CVirtualHost*> CVirtualHost:: vt_virtualHost;
CVirtualHost::CVirtualHost(string &ftpName)
{
	this->ftpName = ftpName;
	lastModified  = time(NULL);
	errorInfo     = "";
	isUsing       = false;
}

int CVirtualHost::GetSecondsDiff()
{
	return time(NULL) - lastModified;
}

bool CVirtualHost::LoadFile()
{
	if(vt_conf.size() != 0)
		return true;
	string path = MakeConfPath(ftpName);
	if(!ReadFile(&vt_conf,path.c_str()))
	{
		char buf[256];
		sprintf(buf,"read confile file %s.conf failed",ftpName.c_str());
		errorInfo = buf;
		vt_conf.clear();
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
		sprintf(buf,"write config file %s.conf failed",ftpName.c_str());
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

vector<string>::iterator CVirtualHost::FindNode(string &node,vector<string> &nodeParam,vector<string>::iterator it)
{
	vector<string> vt_tmp;
	bool exist = false;
	for(; it != vt_conf.end(); it++)
	{
		vt_tmp.clear();
		const char *tmp = (*it).c_str();
		if(!IsNodeStart(tmp))
			continue;
		else
		{
			const char *directiveStart = GetFirstNotSpaceChar(tmp) + 1;
			Split(directiveStart,vt_tmp);
			if(IsEqualString(node,vt_tmp[0]))
			{
				vector<string>::iterator it_param = nodeParam.begin();
				for(; it_param != nodeParam.end(); it_param++)
				{
					if(!StrInVt((*it_param),vt_tmp))
						break;
				}
				if(it_param == vt_tmp.end() && nodeParam.size() != 0)
					exist = true;
				if(nodeParam.size() == 0 && vt_tmp.size() == 0)
					exist = true;
				if(it_param == nodeParam.end())
					exist = true;
			}
			else
				continue;
		}
		if(exist)
			break;
	}
	return it;
}

const char* CVirtualHost::GetFirstNotSpaceChar(const char* line)
{
	while(*line == 32 || *line == 9)
		line++;
	return line;
}

bool CVirtualHost::IsNote(const char* line)
{
	const char *tmp = GetFirstNotSpaceChar(line);
	if(*tmp == '#')
		return true;
	return false;
}

bool CVirtualHost::IsNodeStart(const char* line)
{
	if(IsNote(line))
		return false;
	const char *tmp = GetFirstNotSpaceChar(line);
	if(*tmp == '<')
		return true;
	return false;
}

bool CVirtualHost::IsNodeEnd(const char *line)
{
	if(IsNote(line))
		return false;
	const char *tmp = GetFirstNotSpaceChar(line);
	if(strncmp(tmp,"</",strlen("</")) == 0)
		return true;
	return false;
}

vector<string>::iterator CVirtualHost::FindGlobalDirective(string &directive,string param[],int n,vector<string>::iterator it)
{
	vector<string> vt_tmp;
	bool exist = true;
	bool node  = false;
	for(; it != vt_conf.end(); it++)
	{
		const char* tmp = (*it).c_str();
		if(IsNote(tmp))
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
			{
				continue;
			}
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

vector<string>::iterator CVirtualHost::FindNodeDirective(vector<string>::iterator it, string &directive,vector<string> &vt_param)
{
	if(!IsNodeStart((*it).c_str()))
	{
		//errorInfo = "请传入节点所在行";
		return vt_conf.end();
	}
	it++;
	bool exist = false;
	vector<string> vt_tmp;
	while(!IsNodeEnd((*it).c_str()))
	{
		if(IsNote((*it).c_str()))
		{
			it++;
			continue;
		}
		Split((*it),vt_tmp);
		if(IsEqualString(vt_tmp[0],directive))
		{
			vector<string>::iterator it_param = vt_param.begin();
			for(; it_param != vt_param.end(); it++)
			{
				if(!StrInVt(*it,vt_tmp))
				{
					exist = false;
					break;
				}
			}
			if(it_param == vt_param.end())
			{
				exist = true;
				break;
			}
		}
		else
			it++;
		vt_tmp.clear();
	}
	return it;
}

vector<string>::iterator CVirtualHost::EraseItem(vector<string>::iterator it)
{
	return vt_conf.erase(it);
}

vector<string>::iterator CVirtualHost::AddNode(string &node,vector<string>::iterator it,vector<string> &vt_nodeParam)
{
	string oneRecord = "    ";
	oneRecord.append("<");
	oneRecord.append(node);
	for(int i = 0; i < vt_nodeParam.size(); i++)
	{
		oneRecord.append(" ");
		oneRecord.append(vt_nodeParam[i]);
	}
	oneRecord.append(">");
	oneRecord.append(NEWLINE);
	vt_conf.insert(it,oneRecord);
	it++;
	oneRecord = "    ";
	oneRecord.append("</");
	oneRecord.append(node);
	oneRecord.append(">");
	oneRecord.append(NEWLINE);
	it = vt_conf.insert(it,oneRecord);
	return it;
}

void CVirtualHost::AddDirective(string &directive,vector<string>::iterator it,vector<string> &vt_param,int n)
{
	string oneRecord(n,' ');
	oneRecord.append(directive);
	for(int i = 0; i < vt_param.size(); i++)
	{
		oneRecord.append(" ");
		oneRecord.append(vt_param[i]);
	}
	oneRecord.append(NEWLINE);
	vt_conf.insert(it,oneRecord);
}

CVirtualHost* CVirtualHost::GetVirtualHost(string &fileName)
{
	pthread_mutex_lock(&mutex);
	CVirtualHost *ret = NULL;
	vector<CVirtualHost*>::iterator it = vt_virtualHost.begin();
	for(; it != vt_virtualHost.end(); it++)
	{
		if(IsEqualString(fileName,(*it)->GetFileName()))
		{
			ret = *it;
			break;
		}
	}
	if(ret == NULL)
	{
		ret = new CVirtualHost(fileName);
	}
	if(ret != NULL && ret->IsUsing())
		ret = NULL;
	pthread_mutex_unlock(&mutex);
	return ret;
}

void CVirtualHost::ReleaseVirtualHost(string &fileName)
{
	pthread_mutex_lock(&mutex);
	vector<CVirtualHost*>::iterator it = vt_virtualHost.begin();
	for(; it != vt_virtualHost.end(); it++)
	{
		if(IsEqualString(fileName,(*it)->GetFileName()))
		{
			//(*it)->ResetUsingState();
			delete (*it);
			vt_virtualHost.erase(it);
			break;
		}
	}
	pthread_mutex_unlock(&mutex);
}


