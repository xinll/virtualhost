/*************************************************************************
    > File Name: ServerError.cpp
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年09月13日 星期五 09时10分35秒
 ************************************************************************/

#include"serverError.h"
void MakeErrorDocument(string &insertContent,const string &errorNumStr,const string &errorDocument)
{
	insertContent.append("    ");
	insertContent.append(ERRORHEAD);
	insertContent.append(" ");
	insertContent.append(errorNumStr);
	insertContent.append(" ");
	insertContent.append(errorDocument);
	insertContent.append(NEWLINE);
}

void ChangeError(int error,const string &errorDocument,vector<string> *vt_conf)
{
	vector<string>::iterator it;
	vector<string>::iterator it_recordError = vt_conf->begin(); //记录一个ErrorDocument出现的地方
	bool found = false;
	for(it = vt_conf->begin(); it != vt_conf->end(); it++)
	{
		if((*it).find(ERRORHEAD) != string::npos)
		{
			it_recordError = it;
		}
		if((*it).find(ERRORHEAD) != string::npos && (*it).find("#") == string::npos)
		{
			vector<string> vt_tmp;
			const char *data = (*it).c_str();
			const char *firstCharNotSpace = data;
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
					vt_tmp.push_back(dest);
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
			
			string errorStr = vt_tmp.at(1);
			int errorNumber = atoi(errorStr.c_str());
			if(errorNumber == error)
			{
				string insertContent;
				MakeErrorDocument(insertContent,errorStr,errorDocument);
				it = vt_conf->erase(it);
				vt_conf->insert(it,insertContent);
				found = true;
				break;
			}
		}

	}

	if(!found)
	{
		string insertContent;
		char tmp[256];
		sprintf(tmp,"%d",error);
		string errorNumStr(tmp);
		MakeErrorDocument(insertContent,errorNumStr,errorDocument);
		if(it_recordError == vt_conf->begin())
			it_recordError++;
		vt_conf->insert(it_recordError,insertContent);
	}
}
