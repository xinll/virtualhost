/*************************************************************************
    > File Name: action.h
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年09月29日 星期日 13时24分09秒
 ************************************************************************/

#include<vector>
#include<string>
#include"log.h"
using namespace std;

class CVirtualHost;
class CAction
{
public:
	static bool ProcErrorDocument(vector<pair<string,string> > &vt_param,string &errInfo);

	static bool ProcFilePermission(vector<pair<string,string> > &vt_param,string &errInfo);

	static void DeleteRootDirectory(vector<pair<string,string> >&vt_param,string &errInfo);

	static bool ProcRedirect(vector<pair<string,string> >&vt_param,string &errInfo);

	static bool ProcMineType(vector<pair<string,string> > &vt_param,string &errInfo);

	static bool ProcDirectoryAccess(vector<pair<string,string> > &vt_param,string &errInfo);

	static bool Compress(vector<pair<string,string> > &vt_param,string &errInfo);

	static bool UnCompress(vector<pair<string,string> > &vt_param,string &errInfo);

private:
	static void AddRedirect(string redirectFrom,string redirectTo,CVirtualHost *host);

	static void DeleteRedirect(string redirectFrom,CVirtualHost *host);

	static bool InitEnv(CVirtualHost **virtualHost,string &userName,string &errInfoi,char *category);

	static void DeleteMineType(string &mineType,CVirtualHost *virtualHost);

	static void AddMineType(string &mineType,string &procMethod,CVirtualHost *virtualHost);

	static bool WriteFile(CVirtualHost *virtualHost,string &errInfo,char *category);

	static void DeleteNodeDirective(vector<string>::iterator it,string &directive,vector<string> vt_param,CVirtualHost *virtualHost);

	static bool AddAccess(string &ip,string &dir,CVirtualHost *virtualHost,bool allow);

	static bool AddAccessIpEmpty(string &dir,CVirtualHost *virtualHost,bool allow);

	static bool AddAccessIpNotEmpty(string &dir,CVirtualHost *virtualHost,string &ip,bool allow);

	static bool AddAccessDirEmptyIpEmpty(CVirtualHost *host,bool allow);

	static bool AddAccessDirNotEmptyIpEmpty(CVirtualHost *host,string &dir,bool allow);

	static bool AddAccessDirEmptyIpNotEmpty(CVirtualHost *host,string &ip,bool allow);

	static bool AddAccessDirNotEmptyIpNotEmpty(CVirtualHost *host,string &ip,string &dir,bool allow);
	static bool DirectoryAccess(string &action,string &ip,string &dir,CVirtualHost *virtualHost);
};
