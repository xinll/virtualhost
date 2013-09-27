/*************************************************************************
    > File Name: tools.cpp
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年09月16日 星期一 09时53分12秒
 ************************************************************************/

#include "tools.h"
#include <fstream>
#include <stdlib.h>
#include <string.h>

bool ReadFile(vector<string> *vt_conf,const char *fileName)
{

	fstream f;
	f.open(fileName,ios::in | ios::out);
	if(!f)	
	{
		//打开文件出错,......
		return false;
	}
	
	while(!f.eof())
	{
		string line;
		getline(f,line);
		if(line == NEWLINE)
			continue;
		line.append(NEWLINE);
		vt_conf->push_back(line);
	}
	f.close();
	return true;
}


bool  WriteFile(vector<string> *vt_conf,const char *fileName)
{
	fstream fs;
	fs.open(fileName,ios::out | ios::trunc);
	if(!fs)
		return false;
	vector<string>::iterator it;
	for(it = vt_conf->begin(); it != vt_conf->end(); it++)
	{
		if((*it) != NEWLINE)
			fs << (*it);
	}
	fs.close();
	return true;
}

bool ProcParam(char *param,vector< pair<string,string> > &vt_param)
{
	char *tmp = param;
	char *p = NULL;
	while(p = strstr(tmp,SPLIT))
	{
		if(p == tmp)
		{
			tmp += strlen(SPLIT);
			continue;
		}
		string str(tmp,p - tmp);

		size_t found = str.find("=");
		if(found == string::npos)
		{
			return false;
		}
		else
		{
			string key = str.substr(0,found);
			string value = str.substr(found + 1);
			pair<string,string> m = make_pair(key,value);
			vt_param.push_back(m);
		}
		tmp = p + strlen(SPLIT);
		//add to .\r\n
		if(strstr(tmp,PARAMEND) == tmp)
			break;
	}
	return true;
}

extern string dirPath;
string MakeConfPath(string &ftpName)
{
	string path = dirPath + ftpName + ".conf";
	return path;
}

void Split(string source,vector<string> &result)
{
	const char* data = source.c_str();
	const char* firstCharNotSpace = data;
	while(*data != '\0')
	{
		if((*data == 32 || *data == 9) && data == firstCharNotSpace) //空格
		{
			data++;
			firstCharNotSpace = data;
			continue;
		}
		else if((*data == 32 || *data == 9) && data != firstCharNotSpace)
		{
			string dest(firstCharNotSpace,data - firstCharNotSpace);
			result.push_back(dest);
			data++;
			firstCharNotSpace = data;
			continue;
		}
		else
		{
			data++;
			if(firstCharNotSpace == '\0')
				firstCharNotSpace = data;
		}
	}
}

bool IsEqualString(string first,string second)
{
	return strcasecmp(first.c_str(),second.c_str()) == 0;
}
