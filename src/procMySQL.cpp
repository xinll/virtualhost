/*************************************************************************
    > File Name: ../src/procMySQL.cpp
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年09月24日 星期二 13时42分44秒
 ************************************************************************/
#include "procMySQL.h"
#include "defines.h"
#include "ftp.h"
#include <stdio.h>
#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include "tools.h"
#include <mysql/mysql.h>

bool MySQLBack(vector<pair<string,string> > &vt_param,string &errInfo)
{
	if(vt_param.size() < 5)
	{
		errInfo.append("参数太少");
		errInfo.append(SPLIT);
		return false;
	}
	string ftpUserName,ftpPw,mySQLUserName,mySQLPw,mySQLDataBase;
	int ftpPort = 21;
	string ftpServer = "127.0.0.1";
	string mySQLServer = "127.0.0.1";
	string ftpDir = "/";
	time_t t = time(NULL);
	char buf[100];
	sprintf(buf,"%ld.sql",t);
	string bakFileName(buf);
	int size = vt_param.size();
	for(int i = 2; i < size; i++)
	{
		if(IsEqualString(vt_param[i].first,FTPUSER))
		{
			ftpUserName = vt_param[i].second;
			continue;
		}
		if(IsEqualString(vt_param[i].first,FTPPW))
		{
			ftpPw = vt_param[i].second;
			continue;
		}
		if(IsEqualString(vt_param[i].first,MYSQLUSER))
		{
			mySQLUserName = vt_param[i].second;
			continue;
		}
		if(IsEqualString(vt_param[i].first,MYSQLPW))
		{
			mySQLPw = vt_param[i].second;
			continue;
		}

		if(IsEqualString(vt_param[i].first,MYSQLSERVER))
		{
			mySQLServer = vt_param[i].second;
			continue;
		}
		if(IsEqualString(vt_param[i].first,FTPSERVER))
		{
			ftpServer = vt_param[i].second;
			continue;
		}
		if(IsEqualString(vt_param[i].first,MYSQLBAKNAME))
		{
			bakFileName = vt_param[i].second;
			continue;
		}
		if(IsEqualString(vt_param[i].first,FTPPORT))
		{
			string tmp  = vt_param[i].second;
			ftpPort = atoi(tmp.c_str());
			continue;
		}
		if(IsEqualString(vt_param[i].first,MYSQLBASE))
		{
			mySQLDataBase  = vt_param[i].second;
			continue;
		}
		if(IsEqualString(vt_param[i].first,FTPDIR))
		{
			ftpDir = vt_param[i].second;
			continue;
		}
	}
	if(ftpUserName.empty() || mySQLUserName.empty() || mySQLDataBase.empty())
	{
		//参数错误
		errInfo.append("ftpUserName或mySQLUserName或mySQLDataBase名不合法");
		errInfo.append(SPLIT);
		return false;
	}

	//size_t found = bakFileName.rfind("/");
	string dir = "/tmp";
	/*if(found != string::npos)
	{
		dir = bakFileName.substr(0,found);
		bakFileName = bakFileName.substr(found + 1,bakFileName.size() - found - 1);
	}*/
	chdir(dir.c_str());
	char cmdBuf[1024];
	sprintf(cmdBuf,"mysqldump -u%s -p%s -h%s %s --add-drop-database --database --lock-all-tables --disable-keys > %s",mySQLUserName.c_str(),mySQLPw.c_str(),mySQLServer.c_str(),mySQLDataBase.c_str(),bakFileName.c_str());
    int ret = system(cmdBuf);
	if(ret != -1 && WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
	{
		syslog(LOG_INFO,"备份MySQL脚本成功%s",mySQLDataBase.c_str());
	}
	else
	{
		errInfo.append("备份MySQL脚本失败");
		errInfo.append(SPLIT);
		syslog(LOG_ERR,"备份MySQL脚本失败%s",mySQLDataBase.c_str());
		return false;
	}
	CFTP ftpClient;
	int err = ftpClient.ftp_connect(ftpServer.c_str(),(short)ftpPort);
	if(err)
	{
		//连接ftp错误
		syslog(LOG_ERR,"can't connect the ftp server!!!");
		errInfo.append("连接ftp服务器失败");
		errInfo.append(SPLIT);
		chdir("/");
		return false;
	}
	err = ftpClient.ftp_login(ftpUserName.c_str(),ftpPw.c_str());
	if(err)
	{
		//登陆错误
		syslog(LOG_ERR,"can't login the ftp server!!!");
		errInfo.append("登陆ftp服务器失败");
		errInfo.append(SPLIT);
		chdir("/");
		return false;
	}

	err = ftpClient.ftp_upload(bakFileName.c_str(),ftpDir.c_str(),bakFileName.c_str());
	chdir("/");
	if(err)
	{
		//上传文件错
		syslog(LOG_ERR,"upload file failed!!!");
		errInfo.append("上传文件到ftp服务器失败");
		errInfo.append(SPLIT);
		return false;
	}
	ftpClient.ftp_quit();
	return true;
}

bool MySQLRestore(vector<pair<string,string> > &vt_param,string &errInfo)
{
	string ftpUserName,ftpPw,mySQLUserName,mySQLPw,mySQLDataBase,bakFileName;
	int ftpPort = 21;
	string ftpServer = "127.0.0.1";
	string mySQLServer = "127.0.0.1";
	int size = vt_param.size();
	string ftpDir = "/";
	for(int i = 2; i < size; i++)
	{
		if(IsEqualString(vt_param[i].first,FTPUSER))
		{
			ftpUserName = vt_param[i].second;
			continue;
		}
		if(IsEqualString(vt_param[i].first,FTPPW))
		{
			ftpPw = vt_param[i].second;
			continue;
		}
		if(IsEqualString(vt_param[i].first,MYSQLUSER))
		{
			mySQLUserName = vt_param[i].second;
			continue;
		}
		if(IsEqualString(vt_param[i].first,MYSQLPW))
		{
			mySQLPw = vt_param[i].second;
			continue;
		}

		if(IsEqualString(vt_param[i].first,MYSQLSERVER))
		{
			mySQLServer = vt_param[i].second;
			continue;
		}
		if(IsEqualString(vt_param[i].first,FTPSERVER))
		{
			ftpServer = vt_param[i].second;
			continue;
		}
		if(IsEqualString(vt_param[i].first,MYSQLBAKNAME))
		{
			bakFileName = vt_param[i].second;
			continue;
		}
		if(IsEqualString(vt_param[i].first,FTPPORT))
		{
			string tmp  = vt_param[i].second;
			ftpPort = atoi(tmp.c_str());
			continue;
		}
		if(IsEqualString(vt_param[i].first,MYSQLBASE))
		{
			mySQLDataBase  = vt_param[i].second;
			continue;
		}
		if(IsEqualString(vt_param[i].first,FTPDIR))
		{
			ftpDir = vt_param[i].second;
			continue;
		}
	}
	if(ftpUserName.empty() || mySQLUserName.empty() || bakFileName.empty())
	{
		//参数错误
		errInfo.append("参数不合法");
		errInfo.append(SPLIT);
		return false;
	}
	
	CFTP ftpClient;
	int err = ftpClient.ftp_connect(ftpServer.c_str(),(short)ftpPort);
	if(err)
	{
		//连接ftp错误
		syslog(LOG_ERR,"can't connect the ftp server!!!");
		errInfo.append("连接ftp服务器失败");
		errInfo.append("SPLIT");
		return false;
	}
	err = ftpClient.ftp_login(ftpUserName.c_str(),ftpPw.c_str());
	if(err)
	{
		//登陆错误
		errInfo.append("登陆ftp服务器失败");
		errInfo.append(SPLIT);
		syslog(LOG_ERR,"can't login the ftp server!!!");
		return false;
	}
//	size_t found = bakFileName.rfind("/");
	string dir = "/tmp/";
/*	if(found != string::npos)
	{
		dir = bakFileName.substr(0,found);
		bakFileName = bakFileName.substr(found + 1,bakFileName.size() - found - 1);
	}*/
	err = ftpClient.ftp_cd(ftpDir.c_str());
	if(err)
	{
		errInfo.append("ftp不能打开目录");
		errInfo.append(SPLIT);
		return false;
	}
	chdir(dir.c_str());
	err = ftpClient.ftp_download(bakFileName.c_str(),bakFileName.c_str());

	if(err)
	{
		//上传文件错误
		errInfo.append("ftp下载文件失败");
		errInfo.append(SPLIT);
		syslog(LOG_ERR,"download file failed!!!");
		chdir("/");
		return false;
	}
	ftpClient.ftp_quit();

	char cmdBuf[1024];
	sprintf(cmdBuf,"mysql -u%s -p%s -h%s mysql < %s",mySQLUserName.c_str(),mySQLPw.c_str(),mySQLServer.c_str(),bakFileName.c_str());
	int ret = system(cmdBuf);
	chdir("/");

	if(ret != -1 && WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
	{
		syslog(LOG_INFO,"恢复MySQL数据库成功");
		return true;
	}
	else
	{
		errInfo.append("恢复MySQL数据库失败");
		errInfo.append(SPLIT);
		syslog(LOG_ERR,"恢复MySQL数据库失败");
		return false;
	}
}

//读取数据库大小
long long GetDataBaseSize(string host,string user,string pwd,string db,unsigned int port = 3306)
{
	MYSQL mysql;
	
	mysql_init(&mysql);
	
	if(!mysql_real_connect(&mysql,host.c_str(),user.c_str(),pwd.c_str(),db.c_str(),port,NULL,0))
	{
		;//连接数据库失败
	}

	string query = "SHOW TABLE STATUS";
	if(!mysql_query(&mysql,query.c_str()))
	{
		mysql_close(&mysql);
		return 0;
	}
	MYSQL_RES *result = mysql_store_result(&mysql);
	if(result == NULL)
	{
		mysql_close(&mysql);
		return 0;
	}

	long long size = 0;
	MYSQL_ROW row;
	MYSQL_FIELD *fields = mysql_fetch_fields(result);
	
	while((row = mysql_fetch_row(result)))
	{
		size += strtoll(row[6],NULL,0);
		size += strtoll(row[8],NULL,0);
	}
	mysql_free_result(result);
	mysql_close(&mysql);
	return size;
}

/*extern pthread_mutex_t mutex;
void LimitMySQLSize()
{
	pthread_mutex_lock(&mutex);
	vector<string> vt_conf;
	ReadFile(&vt_conf,);
	pthread_mutex_unlock(&mutex);
	vector<string>::iterator it = vt_conf.begin();
	vector<string>
	for(; it != vt_conf.end(); it++)
	{
		
	}
}*/
