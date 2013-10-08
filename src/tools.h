/*************************************************************************
    > File Name: tools.h
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年09月16日 星期一 09时50分49秒
 ************************************************************************/

#include <vector>
#include <string>
#include "defines.h"
using namespace std;

bool ReadFile(vector<string> *vt_conf,const char *fileName);

bool WriteFile(vector<string> *vt_conf, const char *fileName);

bool ProcParam(char *param,vector<pair<string,string> > &vt_param);

string MakeConfPath(string &ftpName);

void Split(string source,vector<string> &result);

bool IsEqualString(string first,string second);

bool RestoreConf(string &userName);

bool BakConf(string &userName);

bool StrInVt(string &str,vector<string> &vt);

bool UpLoadFile(const char* ftpServer,const char* ftpUser,const char* ftpPwd,const char* file,const char* dir);

void RmDir(const char* path);
