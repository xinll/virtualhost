/*************************************************************************
    > File Name: changePermission.cpp
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年09月13日 星期五 10时03分27秒
 ************************************************************************/

#include "changePermission.h"
#include <string.h>
void MakeFilePermission(string &insertContent,int permission,const string& file,bool needFile = true)
{
	if(needFile)
	{
		insertContent.append("        <Files ~ \"");
		insertContent.append(file);
		insertContent.append("\">");
		insertContent.append(NEWLINE);
	}
	switch(permission)
	{
		case PERMISSION_ALLOW:		
		{
			insertContent.append("            Order deny,allow");
			insertContent.append(NEWLINE);
			insertContent.append("            allow from all");
			insertContent.append(NEWLINE);
			break;
		}
		case PERMISSION_DENY:
		{
			insertContent.append("            Order allow,deny");
			insertContent.append(NEWLINE);
			insertContent.append("            deny from all");
			insertContent.append(NEWLINE);
			break;
		}
	}
	if(needFile)
	{
		insertContent.append("        </Files>");
		insertContent.append(NEWLINE);
	}
}

void ChangePermission(const string &dir,const string &file,int permission,vector<string> &vt_conf)
{
	
	vector<string>::iterator it = vt_conf.begin();
	if(dir.empty())
	{
		bool fileExist = false;
		bool dirExist = false;
		//add to slove directoryroot
		string directoryRoot;
		for(;it != vt_conf.end();it++)
		{
			size_t found;
			if((found=(*it).find("DocumentRoot")) != string::npos)
			{
				int length = (*it).size();
				for(found += strlen("DocumentRoot");found < length;found++)
				{
					if((*it)[found] != 32 && (*it)[found] != 9 )
					{

						directoryRoot = (*it).substr(found,length - found);
					}			
				}
				break;
			}
		}
		for(it = vt_conf.begin();it != vt_conf.end(); it++)
		{
			if((*it).find("<Directory") != string::npos && (*it).find(directoryRoot) == string::npos) //the second added to slove directory
			{
				dirExist = true;
				continue;
			}
			if((*it).find("</Directory>") == string::npos && dirExist)
				continue;
			else
				dirExist = false;
			if((*it).find("<Files") != string::npos && (*it).find(file) != string::npos)
			{
				fileExist = true;
				break;
			}
		}
		if(!fileExist)
		{
			string insertContent;
			MakeFilePermission(insertContent,permission,file);
			it = vt_conf.begin();
			it++;
			vt_conf.insert(it,insertContent);
		}
		else
		{
			for(it++; it != vt_conf.end(); it++)
			{
				if((*it).find("Order") != string::npos)
				{
					it = vt_conf.erase(it);
				}
				if((*it).find("allow") != string::npos)
				{
					it = vt_conf.erase(it);
				}
				if((*it).find("deny") != string::npos)
				{
					it = vt_conf.erase(it);
				}
				if((*it).find("</Files>") != string::npos)
					break;
			}
			string insertContent;
			MakeFilePermission(insertContent,permission,file,false);
			vt_conf.insert(it,insertContent);
		}
	}
	else
	{
		bool dirExist = false;
		for(; it != vt_conf.end(); it++)
		{
			if((*it).find("<Directory") != std::string::npos)
			{
				//找到了相关的目录
				if((*it).find(dir) != std::string::npos)
				{
					dirExist = true;
					break;
				}
				else
					continue;
			}
		}

		if(!dirExist)
		{
			string insertContent = "    <Directory ";
			insertContent.append("\"");
			insertContent.append(dir);
			insertContent.append("\"");
			insertContent.append(">");
			insertContent.append(NEWLINE);
			MakeFilePermission(insertContent,permission,file);
			insertContent.append("    </Directory>");
			insertContent.append(NEWLINE);
			it = vt_conf.begin();
			it++;
			vt_conf.insert(it,insertContent);
		}
		else
		{
			bool fileExist = false;
			for(it++; it != vt_conf.end(); it++)
			{
				if((*it).find("</Directory>") != string::npos)
				{
					break;
				}
				if((*it).find("<Files") != string::npos && (*it).find(file) != string::npos)
				{
					fileExist = true;
					break;
				}
			}
			if(!fileExist)
			{
				string insertContent;
				MakeFilePermission(insertContent,permission,file);
				vt_conf.insert(it,insertContent);
			}
			else
			{
				for(it++; it != vt_conf.end(); it++)
				{
					if((*it).find("Order") != string::npos)
					{
						it = vt_conf.erase(it);
					}
					if((*it).find("allow") != string::npos)
					{
						it = vt_conf.erase(it);
					}
					if((*it).find("deny") != string::npos)
					{
						it = vt_conf.erase(it);
					}
					if((*it).find("</Files>") != string::npos)
						break;
				}
				string insertContent;
				MakeFilePermission(insertContent,permission,file,false);
				vt_conf.insert(it,insertContent);
			}
		}
	}
}
