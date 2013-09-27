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
	vector<string>::iterator it;
	time_t         lastModified;
	string         ftpName;
	string         errorInfo;
public:
	CVirtualHost(string &ftpName);
	int  GetSecondsDiff();
	bool LoadFile();
	bool SaveFile();
	int  FindNode(string &node,string &nodeParam);
	vector<string>::iterator&  FindGlobalDirective(string &directive);
	int  FindNodeDirective();
	bool AddNode(string &node,vector<string>::iterator &it,string &nodeParam);
	bool AddDirective(string &directive,vector<string>::iterator &it,string nodeParam[]);
	
	vector<string>::iterator GetIterator(int offset = 0);
	vector<string>::iterator GetEndIterator();
	string GetLastErrorStr() {return errorInfo;}
};
