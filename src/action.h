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
private:
	static void AddRedirect(string &redirectFrom,string &redirectTo,CVirtualHost *host);

	static void DeleteRedirect(string &redirectFrom,CVirtualHost *host);


	static bool InitEnv(CVirtualHost **virtualHost,string &userName,string &errInfoi,zlog_category_t *c);

	static void DeleteMineType(string &mineType,CVirtualHost *virtualHost);

	static void AddMineType(string &mineType,string &procMethod,CVirtualHost *virtualHost);

	static bool WriteFile(CVirtualHost *virtualHost,string &errInfo,zlog_category_t *c);
};
