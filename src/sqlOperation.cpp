/*************************************************************************
    > File Name: sqlOperation.cpp
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年10月18日 星期五 17时58分37秒
 ************************************************************************/

#include "sqlOperation.h"
#include "procMySQL.h"
#include "tools.h"
#include "config.h"

bool ProcSql(vector<pair<string,string> > vt_param,string &errInfo)
{
	if(vt_param.size() < 2)
	{
		errInfo.append("too less param. ");
		return false;
	}
	pair<string,string> p = vt_param[1];
	if(!IsEqualString(p.first,NFUNC))
	{
		errInfo.append("error param:nfunc. ");
		return false;
	}
	string value = p.second;
	
	 if(IsEqualString(value,MYSQLBACK))
	{
		return MySQLBack(vt_param,errInfo);
	}
	else if(IsEqualString(value,MYSQLRESTORE))
	{
		return MySQLRestore(vt_param,errInfo);
	}
	else if(IsEqualString(value,LIMITMYSQLSIZE))
	{
		return RecordLimit(vt_param,errInfo,false);
	}
	else if(IsEqualString(value,CHECKMYSQLSIZESTATE))
	{
		return RecordLimit(vt_param,errInfo,true);//立即检查
	}
	else if(IsEqualString(value,MYSQLGETSIZE))
	{
		Config config;
		config.LoadConfigFile();
		string pwd = config.GetValue("MYSQLPWD");
	    long long size = GetDataBaseSize("localhost","root",pwd,vt_param[2].second);
		char result[2014];
		sprintf(result,"%lld",size);
		errInfo = result;	
	}
	else
	{
		errInfo.append("unknow operation:");
		errInfo.append(value);
		return false;
	}
	return true;
}

