/*************************************************************************
    > File Name: action.h
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年09月29日 星期日 13时24分09秒
 ************************************************************************/

#include<vector>
#include<string>
using namespace std;
class CAction
{
public:
	static bool ProcErrorDocument(vector<pair<string,string> > &vt_param,string &errInfo);

	static bool ProcFilePermission(vector<pair<string,string> > &vt_param,string &errInfo);

	static void DeleteRootDirectory(vector<pair<string,string> >&vt_param,string &errInfo);
};
