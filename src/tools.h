/*************************************************************************
    > File Name: tools.h
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年09月16日 星期一 09时50分49秒
 ************************************************************************/
#ifndef _TOOLS_H
#define _TOOLS_H
#include <vector>
#include <string>
#include "defines.h"
#include "log.h"

using namespace std;

bool ReadFile(vector<string> *vt_conf,const char *fileName);

bool WriteFile(vector<string> *vt_conf, const char *fileName);

bool ProcParam(char *param,vector<pair<string,string> > &vt_param);

string MakeConfPath(string &ftpName);

string GetEnvVar(string key);

void Split(string source,vector<string> &result);

bool IsEqualString(string first,string second);

bool RestoreConf(string &userName);

bool BakConf(string &userName);

bool StrInVt(string &str,vector<string> &vt);

bool UpLoadFile(const char* ftpServer,const char* ftpUser,const char* ftpPwd,const char* file,const char* dir);

void RmDir(const char* path);

void WriteParam(char *c,vector<pair<string,string> > &vt_param,string success);

void SplitByComas(string &source,vector<string> &result,char split = ',');

bool CheckParam(vector<pair<string,string> > &vt_param,int count,string &errInfo);
#endif
