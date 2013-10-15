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
#include "log.h"
#include <sys/stat.h>
#include "config.h"

zlog_category_t *c;
extern pthread_mutex_t mutex;

bool MySQLBack(vector<pair<string,string> > &vt_param,string &errInfo)
{
	pthread_mutex_lock(&mutex);
	if(!c)
	{
		c = GetCategory("backupMysql");
	}
	pthread_mutex_unlock(&mutex);

	WriteParam(c,vt_param,"");

	if(vt_param.size() < 5)
	{
		errInfo.append("too less param.");
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
		errInfo.append("ftpUserName or mySQLUserName or mySQLDataBase not valid.");
		WriteLog(c,ERROR,"ftpUserName or mySQLUserName or mySQLDataBase not valid");
		return false;
	}

	string dir = "/tmp";
	chdir(dir.c_str());
	char cmdBuf[1024];
	sprintf(cmdBuf,"mysqldump -u%s -p%s -h%s %s --add-drop-database --database --lock-all-tables --disable-keys > %s",mySQLUserName.c_str(),mySQLPw.c_str(),mySQLServer.c_str(),mySQLDataBase.c_str(),bakFileName.c_str());
    int ret = system(cmdBuf);
	if(ret != -1 && WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
	{
		char tmp[256];
		sprintf(tmp,"backup MySQL %s success",mySQLDataBase.c_str());
		WriteLog(c,INFO,tmp);
	}
	else
	{
		errInfo.append("backup MySQL %s failed",mySQLDataBase.c_str());
	
		char tmp[256];
		sprintf(tmp,"backup MySQL %s failed",mySQLDataBase.c_str());
		WriteLog(c,INFO,tmp);
		return false;
	}
	CFTP ftpClient;
	int err = ftpClient.ftp_connect(ftpServer.c_str(),(short)ftpPort);
	if(err)
	{
		//连接ftp错误
		errInfo.append("can't connect the ftp server");
		chdir("/");
		return false;
	}
	err = ftpClient.ftp_login(ftpUserName.c_str(),ftpPw.c_str());
	if(err)
	{
		//登陆错误
		errInfo.append("can't login the ftp server");
		chdir("/");
		return false;
	}

	err = ftpClient.ftp_upload(bakFileName.c_str(),ftpDir.c_str(),bakFileName.c_str());
	chdir("/");
	if(err)
	{
		//上传文件错
		errInfo.append("upload the file failed");
		return false;
	}
	ftpClient.ftp_quit();
	WriteParam(c,vt_param,"success");
	return true;
}

bool MySQLRestore(vector<pair<string,string> > &vt_param,string &errInfo)
{
	pthread_mutex_lock(&mutex);
	if(!c)
	{
		c = GetCategory("backupMysql");
	}
	pthread_mutex_unlock(&mutex);
	WriteParam(c,vt_param,"");
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
		errInfo.append("the param is not valid.");
		WriteLog(c,ERROR,"ftpUserName or mySQLUserName or bakFileName not valid");
		return false;
	}
	
	CFTP ftpClient;
	int err = ftpClient.ftp_connect(ftpServer.c_str(),(short)ftpPort);
	if(err)
	{
		//连接ftp错误
		errInfo.append("can't connect the ftp server.");
		return false;
	}
	err = ftpClient.ftp_login(ftpUserName.c_str(),ftpPw.c_str());
	if(err)
	{
		//登陆错误
		errInfo.append("can't login the ftp server.");
		return false;
	}
	string dir = "/tmp/";
	err = ftpClient.ftp_cd(ftpDir.c_str());
	if(err)
	{
		errInfo.append("can't open the directory");
		return false;
	}
	chdir(dir.c_str());
	err = ftpClient.ftp_download(bakFileName.c_str(),bakFileName.c_str());

	if(err)
	{
		//上传文件错误
		errInfo.append("can't download the file.");
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
		WriteLog(c,INFO,"success to restore the database");
		WriteParam(c,vt_param,"success");
		return true;
	}
	else
	{
		errInfo.append("failed to restore the database");
		WriteParam(c,vt_param,"failed");
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
	if(mysql_query(&mysql,query.c_str()))
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

bool LimitMySQLSize(string host,string user,string pwd,string ftpName,string db,long long maxsize)
{

	MYSQL mysql;
	
	mysql_init(&mysql);
	
	if(!mysql_real_connect(&mysql,host.c_str(),user.c_str(),pwd.c_str(),db.c_str(),0,NULL,0))
	{
		;//连接数据库失败
	}

	string query = "SHOW TABLE STATUS";
	if(mysql_query(&mysql,query.c_str()))
	{
		mysql_close(&mysql);
		return false;
	}
	MYSQL_RES *result = mysql_store_result(&mysql);
	if(result == NULL)
	{
		mysql_close(&mysql);
		return false;
	}

	long long size = 0;
	MYSQL_ROW row;
	MYSQL_FIELD *fields = mysql_fetch_fields(result);
	
	while((row = mysql_fetch_row(result)))
	{
		if(row[6] != NULL)
			size += strtoll(row[6],NULL,0);
		if(row[8] != NULL)
			size += strtoll(row[8],NULL,0);
	}
	mysql_free_result(result);
	
	bool success = false;
	if(size/1024 >= maxsize) //转化为M
	{
		char tmpName[256];
		char tmpdb[256];
		mysql_real_escape_string(&mysql,tmpName,ftpName.c_str(),ftpName.size());
		mysql_real_escape_string(&mysql,tmpdb,db.c_str(),db.size());
		char tmpQuery[1024];
		sprintf(tmpQuery,"REVOKE INSERT, UPDATE ON %s.* from %s",tmpdb,tmpName);
		if(mysql_query(&mysql,tmpQuery) == 0)
		{
			strcat(tmpQuery," success");
			WriteLog(c,INFO,tmpQuery);
			success = true;
		}
		else
		{
			strcat(tmpQuery," failed");
			WriteLog(c,ERROR,tmpQuery);
		}
	}
	else
	{
		char tmpName[256];
		char tmpdb[256];
		mysql_real_escape_string(&mysql,tmpName,ftpName.c_str(),ftpName.size());
		mysql_real_escape_string(&mysql,tmpdb,db.c_str(),db.size());
		char tmpQuery[1024];
		sprintf(tmpQuery,"GRANT INSERT, UPDATE ON %s.* to %s",tmpdb,tmpName);
		if(mysql_query(&mysql,tmpQuery) == 0)
		{
			strcat(tmpQuery," success");
			WriteLog(c,INFO,tmpQuery);
			success = true;
		}
		else
		{
			strcat(tmpQuery," failed");
			WriteLog(c,ERROR,tmpQuery);
		}
	}
	mysql_close(&mysql);
	return success;
}

bool RecordLimit(vector<pair<string,string> >&vt_param,string &errInfo,bool check)
{
	struct stat buf;
	static vector<string> vt_conf;

	string ftpName,db,size;
	string ip = "127.0.0.1";
	int length = vt_param.size();
	for(int i = 2; i < length; i++)
	{
		if(IsEqualString(vt_param[i].first,MYSQLADDR))
		{
			ip = vt_param[i].second;
			continue;
		}
		if(IsEqualString(vt_param[i].first,DBNAME))
		{
			db = vt_param[i].second;
			continue;
		}
		if(IsEqualString(vt_param[i].first,MYSQLSIZE))
		{
			size = vt_param[i].second;
			continue;
		}
		if(IsEqualString(vt_param[i].first,USERNAME))
		{
			ftpName = vt_param[i].second;
			continue;
		}
	}

	pthread_mutex_lock(&mutex);
	if(c == NULL)
	{
		c = GetCategory("backupMysql");
	}
	pthread_mutex_unlock(&mutex);

	WriteParam(c,vt_param,"");
	if(vt_param.size() < 3 && !check)
	{
		errInfo.append("too less params");
		return false;
	}
	else if(vt_param.size() < 2 && check)
	{
		errInfo.append("too less params");
		return false;
	}
	if(check)
	{
		pthread_mutex_lock(&mutex);
		if(vt_conf.size() == 0)
		{
			if(!ReadFile(&vt_conf,"/usr/local/apache_conf/mysql.size"))
			{
				errInfo.append("read /usr/local/apache_conf/mysql.size failed");
				WriteLog(c,ERROR,"read /usr/local/apache_conf/mysql.size failed");
				pthread_mutex_unlock(&mutex);
				WriteParam(c,vt_param,"failed");
				return false;
			}
		}
		vector<string>::iterator it = vt_conf.begin();
		vector<string> vt_tmp;
		bool success = false;
		for(;it != vt_conf.end();it++)
		{
			vt_tmp.clear();
			Split((*it),vt_tmp);
			if(vt_tmp.size() < 4)
			{
				continue;
			}
			if(vt_tmp[0].compare(ip) ==0 && vt_tmp[1].compare(ftpName) == 0 && vt_tmp[2].compare(db) == 0)
			{
				long long dbSize = strtoll(vt_tmp[3].c_str(),NULL,10);
				
				Config config;
				config.LoadConfigFile();
				string pwd = config.GetValue("MYSQLPWD");
				success = LimitMySQLSize(vt_tmp[0],"root",pwd,vt_tmp[1],vt_tmp[2],dbSize);
				break;
			}
		}
		pthread_mutex_unlock(&mutex);
		if(!success)
		{
			errInfo.append("revoke or grant failed");
			WriteParam(c,vt_param,"failed");
		}
		else
		{
			WriteParam(c,vt_param,"success");
		}
		return success;
	}
	else
	{
		pthread_mutex_lock(&mutex);
		int ret = stat("/usr/local/apache_conf/mysql.size",&buf);
		string param;
		param.append(ip);
		param.append(" ");
		param.append(ftpName);
		param.append(" ");
		param.append(db);
		param.append(" ");
		param.append(size);
		param.append(NEWLINE);
		if(ret != 0)
		{
			FILE *fp = fopen("/usr/local/apache_conf/mysql.size","a+");
			if(NULL == fp)
			{
				pthread_mutex_unlock(&mutex);
				return false;
			}
			fwrite(param.c_str(),param.size(),1,fp);
			vt_conf.push_back(param);
		}
		else
		{
			if(vt_conf.size() == 0)
			{
				if(!ReadFile(&vt_conf,"/usr/local/apache_conf/mysql.size"))
				{
					errInfo.append("read /usr/local/apache_conf/mysql.size failed");
					WriteLog(c,ERROR,"read /usr/local/apache_conf/mysql.size failed");
					pthread_mutex_unlock(&mutex);
					return false;
				}
			}
			vector<string> vt_tmp;
			vector<string>::iterator it = vt_conf.begin();
			for(;it != vt_conf.end();)
			{
				vt_tmp.clear();
				Split((*it),vt_tmp);
				if(vt_tmp.size() < 4)
				{
					it++;
					continue;
				}
				if(vt_tmp[0].compare(ip) ==0 && vt_tmp[1].compare(ftpName) == 0 && vt_tmp[2].compare(db) == 0)
				{
					vt_conf.erase(it);
					continue;
				}
				else
					it++;
			}
			vt_conf.push_back(param);
			if(!WriteFile(&vt_conf,"/usr/local/apache_conf/mysql.size"))
			{
				errInfo.append("write /usr/local/apache_conf/mysql.size failed");
				WriteLog(c,ERROR,"write /usr/local/apache_conf/mysql.size failed");	
				pthread_mutex_unlock(&mutex);
				return false;
			}
		}
		pthread_mutex_unlock(&mutex);
		WriteParam(c,vt_param,"success");
		return true;
	}
}
