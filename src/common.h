/*************************************************************************
    > File Name: common.h
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年09月27日 星期五 13时26分25秒
 ************************************************************************/

#include <time.h>
#include <string>
#include <iostream>
#include <vector>

using namespace std;

class CVirtualHost
{
private:
	vector<string> vt_conf;
	time_t         lastModified;
	string         ftpName;
	string         errorInfo;
	bool           isUsing;
	static         vector<CVirtualHost*> vt_virtualHost;


private:
	CVirtualHost(string &ftpName);
public:
	int  GetSecondsDiff();
	bool LoadFile();
	bool SaveFile();
	vector<string>::iterator  FindNode(string &node,vector<string> &nodeParam,vector<string>::iterator it);
	vector<string>::iterator  FindGlobalDirective(string &directive,string param[],int n,vector<string>::iterator it);
	vector<string>::iterator  FindNodeDirective(vector<string>::iterator it,string &directive,vector<string> &vt_param);
	vector<string>::iterator AddNode(string &node,vector<string>::iterator it,vector<string> &vt_nodeParam);
	void AddDirective(string &directive,vector<string>::iterator it,vector<string> &vt_param,int n);
	
	vector<string>::iterator EraseItem(vector<string>::iterator it);
	vector<string>::iterator GetIterator(int offset = 0);
	vector<string>::iterator GetEndIterator();
	string GetLastErrorStr() {return errorInfo;}
	string GetFileName()     {return ftpName;}

	static CVirtualHost* GetVirtualHost(string &fileName);
	static void ReleaseVirtualHost(string &fileName);
	bool   IsUsing()          {return isUsing;}
	void   ResetUsingState()  {isUsing = false;}
	bool   IsNote(const char *line);
	const char* GetFirstNotSpaceChar(const char* line);
	bool   IsNodeStart(const char *line);
	bool   IsNodeEnd(const char *line);
};
