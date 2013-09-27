/*************************************************************************
    > File Name: hostOperation.h
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年09月16日 星期一 10时10分44秒
 ************************************************************************/

#include <iostream>
#include <string>
#include "tools.h"
#include "../src/serverError.h"
#include "../src/changePermission.h"
#include "procMySQL.h"

using namespace std;

bool ProcHost(vector<pair<string,string> > vt_param,string &errInfo);
bool BakConf(string &userName);
