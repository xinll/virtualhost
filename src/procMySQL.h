/*************************************************************************
    > File Name: ../src/procMySQL.h
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年09月24日 星期二 13时40分04秒
 ************************************************************************/

#include <vector>
#include <string>
using namespace std;

bool MySQLBack(vector<pair<string,string> > &vt_param,string &errInfo);

bool MySQLRestore(vector<pair<string,string> > &vt_param,string &errInfo);

bool LimitMySQLSize(string host,string user,string pwd,string ftpName,string db,long long maxsize);


bool RecordLimit(vector<pair<string,string> >&vt_param,string &errInfo,bool check);
