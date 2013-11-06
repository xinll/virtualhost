/*************************************************************************
    > File Name: compress.h
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年11月06日 星期三 09时20分22秒
 ************************************************************************/
#ifndef _COMPRESS_H
#define _COMPRESS_H
#include<vector>
#include<string>
using namespace std;

bool Compress(vector<pair<string,string> > &vt_param, string &errInfo);

bool UnCompress(vector<pair<string,string> > &vt_param, string &errInfo);
#endif
