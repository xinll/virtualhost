/*************************************************************************
    > File Name: ../src/procMySQL.cpp
    > Author: xinll
    > Mail: liangliangxinxin@yeah.net
    > Created Time: 2013年09月24日 星期二 13时42分44秒
 ************************************************************************/
#include "procMySQL.h"
#include "defines.h"
#include "ftp.h"
#include <string.h>
#include <stdlib.h>
#include "tools.h"
#include <mysql/mysql.h>
#include "log.h"
#include <sys/stat.h>
#include "config.h"

static char *category = "backupMysqlLog";
extern pthread_mutex_t mutex;

bool MySQLBack(vector<pair<string,string> > &vt_param,string &errInfo)
{
	WriteParam(category,vt_param,"");

	if(vt_param.size() < 5)
	{
		errInfo.append("too less param.");
		return false;
	}
	string ftpUserName = GetValue(FTPUSER,vt_param);
	string ftpPw = GetValue(FTPPW,vt_param);
	string mySQLUserName =GetValue(MYSQLUSER,vt_param);
	string mySQLPw = GetValue(MYSQLPW,vt_param);

	int ftpPort = 21;
	string ftpServer = GetValue(FTPSERVER,vt_param);
	string mySQLServer = "127.0.0.1";
	string ftpDir = GetValue(FTPDIR,vt_param);
	string bakFileName = GetValue(MYSQLBAKNAME,vt_param);
	string strPort = GetValue(FTPPORT,vt_param);
	if(ValidateParamEmpty(strPort.c_str()))
	{
		ftpPort = atoi(strPort.c_str());
	}

	string mySQLDataBase = mySQLUserName;

	if(ftpUserName.empty() || mySQLUserName.empty() || mySQLDataBase.empty())
	{
		//参数错误
		errInfo.append("ftpUserName or mySQLUserName or mySQLDataBase not valid.");
		WriteLog(category,ERROR,"ftpUserName or mySQLUserName or mySQLDataBase not valid");
		return false;
	}

	string dir = "/tmp";
	chdir(dir.c_str());
	char cmdBuf[1024];
	sprintf(cmdBuf,"mysqldump -u%s -p%s -h%s %s > \'%s\'",mySQLUserName.c_str(),mySQLPw.c_str(),mySQLServer.c_str(),mySQLDataBase.c_str(),bakFileName.c_str());
    int ret = system(cmdBuf);
	if(ret != -1 && WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
	{
		char tmp[256];
		sprintf(tmp,"backup MySQL %s success",mySQLUserName.c_str());
		WriteLog(category,INFO,tmp);
	}
	else
	{
		char tmp[256];
		sprintf(tmp,"backup MySQL %s failed",mySQLUserName.c_str());
		errInfo.append(tmp);
		WriteLog(category,INFO,tmp);
		return false;
	}
	CFTP ftpClient;
	int err = ftpClient.ftp_connect(ftpServer.c_str(),(short)ftpPort);
	if(err)
	{
		//连接ftp错误
		errInfo.append("can't connect the ftp server");
		unlink(bakFileName.c_str());
		chdir("/");
		return false;
	}
	err = ftpClient.ftp_login(ftpUserName.c_str(),ftpPw.c_str());
	if(err)
	{
		//登陆错误
		errInfo.append("can't login the ftp server");
		unlink(bakFileName.c_str());
		chdir("/");
		return false;
	}

	err = ftpClient.ftp_upload(bakFileName.c_str(),ftpDir.c_str(),bakFileName.c_str());
	unlink(bakFileName.c_str());
	chdir("/");
	if(err)
	{
		//上传文件错
		errInfo.append("upload the file failed");
		return false;
	}
	ftpClient.ftp_quit();
	WriteParam(category,vt_param,"success");
	return true;
}

bool MySQLRestore(vector<pair<string,string> > &vt_param,string &errInfo)
{
	WriteParam(category,vt_param,"");
	int ftpPort = 21;
	string mySQLServer = "127.0.0.1";
	
	string ftpUserName = GetValue(FTPUSER,vt_param);
	string ftpPw = GetValue(FTPPW,vt_param);
	string mySQLUserName = GetValue(MYSQLUSER,vt_param);
	string mySQLPw = GetValue(MYSQLPW,vt_param);
	string ftpServer = GetValue(FTPSERVER,vt_param);
	string bakFileName = GetValue(MYSQLBAKNAME,vt_param);
	string strPort = GetValue(FTPPORT,vt_param);
	if(ValidateParamEmpty(strPort.c_str()))
	{
		ftpPort = atoi(strPort.c_str());
	}

	string ftpDir = GetValue(FTPDIR,vt_param);
	string dbStrSize = GetValue(MYSQLSIZE,vt_param);
	
	if(ftpUserName.empty() || mySQLUserName.empty() || bakFileName.empty())
	{
		//参数错误
		errInfo.append("the param is not valid.");
		WriteLog(category,ERROR,"ftpUserName or mySQLUserName or bakFileName not valid");
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
	sprintf(cmdBuf,"mysql -u%s -p%s -h%s %s < \'%s\'",mySQLUserName.c_str(),mySQLPw.c_str(),mySQLServer.c_str(),mySQLUserName.c_str(),bakFileName.c_str());
	int ret = system(cmdBuf);

	unlink(bakFileName.c_str());
	chdir("/");

	if(ret != -1 && WIFEXITED(ret) && WEXITSTATUS(ret) == 0)
	{
		WriteLog(category,INFO,"success to restore the database");
		WriteParam(category,vt_param,"success");
		if(!dbStrSize.empty())
		{
			long long dbSize = strtoll(dbStrSize.c_str(),NULL,10);
			string pwd = GetEnvVar("MYSQLPWD");
			if(!LimitMySQLSize("localhost","root",pwd,mySQLUserName,mySQLUserName,dbSize))
			{
				errInfo.append("limit the size failed");
				return false;
			}
		}
		return true;
	}
	else
	{
		errInfo.append("failed to restore the database");
		WriteParam(category,vt_param,"failed");
		return false;
	}
}

//读取数据库大小
long long GetDataBaseSize(string host,string user,string pwd,string db,string &errInfo,unsigned int port)
{
	MYSQL mysql;
	
	mysql_init(&mysql);
	
	if(!mysql_real_connect(&mysql,host.c_str(),user.c_str(),pwd.c_str(),db.c_str(),port,NULL,0))
	{
		;//连接数据库失败
		errInfo.append("can't connect the mysql server!!!");
		char err[1024];
		sprintf(err,"can't connect the mysql server:%s",mysql_error(&mysql));
		WriteLog(category,ERROR,err);
		return -1;
	}

	string query = "SHOW TABLE STATUS";
	if(mysql_query(&mysql,query.c_str()))
	{
		errInfo.append("can't execute sql");
		mysql_close(&mysql);
		return -1;
	}
	MYSQL_RES *result = mysql_store_result(&mysql);
	if(result == NULL)
	{
		mysql_close(&mysql);
		return -1;
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
	mysql_close(&mysql);
	return size/(1024*1024);
}

bool LimitMySQLSize(string host,string user,string pwd,string ftpName,string db,long long maxsize)
{

	MYSQL mysql;
	
	mysql_init(&mysql);
	
	if(!mysql_real_connect(&mysql,host.c_str(),user.c_str(),pwd.c_str(),db.c_str(),0,NULL,0))
	{
		char err[1024];
		sprintf(err,"can't connect the mysql server:%s",mysql_error(&mysql));
		WriteLog(category,ERROR,err);
		return false;
	}

	string query = "SHOW TABLE STATUS";
	if(mysql_query(&mysql,query.c_str()))
	{
		mysql_close(&mysql);
		WriteLog(category,ERROR,"SHOW TABLE STATUS FAILED");
		return false;
	}
	MYSQL_RES *result = mysql_store_result(&mysql);
	if(result == NULL)
	{
		WriteLog(category,ERROR,"Store Result Failed");
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
	if(size/(1024*1024) >= maxsize) //转化为M
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
			WriteLog(category,INFO,tmpQuery);
			success = true;
		}
		else
		{
			strcat(tmpQuery," failed");
			WriteLog(category,ERROR,tmpQuery);
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
			WriteLog(category,INFO,tmpQuery);
			success = true;
		}
		else
		{
			char err[256];
			sprintf(err,"%s",mysql_error(&mysql));
			WriteLog(category,ERROR,err);
			strcat(tmpQuery," failed");
			WriteLog(category,ERROR,tmpQuery);
		}
	}
	mysql_close(&mysql);
	return success;
}

bool RecordLimit(vector<pair<string,string> >&vt_param,string &errInfo,bool check)
{
	struct stat buf;
	static vector<string> vt_conf;

	string db = GetValue(DBNAME,vt_param);
	string size = GetValue(MYSQLSIZE,vt_param);
	string ip = "localhost";
	
	WriteParam(category,vt_param,"");
	if(vt_param.size() < 4 && !check)
	{
		errInfo.append("too less params.");
		return false;
	}
	else if(vt_param.size() < 3 && check)
	{
		errInfo.append("too less params.");
		return false;
	}
	if(check)
	{
		pthread_mutex_lock(&mutex);
		if(vt_conf.size() == 0)
		{
			if(!ReadFile(&vt_conf,"/usr/local/apache_conf/mysql.size"))
			{
				errInfo.append("read /usr/local/apache_conf/mysql.size failed.");
				WriteLog(category,ERROR,"read /usr/local/apache_conf/mysql.size failed");
				pthread_mutex_unlock(&mutex);
				WriteParam(category,vt_param,"failed");
				return false;
			}
		}
		pthread_mutex_unlock(&mutex);
		vector<string>::iterator it = vt_conf.begin();
		vector<string> vt_tmp;
		bool success = false;
		for(;it != vt_conf.end();it++)
		{
			vt_tmp.clear();
			Split((*it),vt_tmp);
			if(vt_tmp.size() < 2)
			{
				continue;
			}
			if(vt_tmp[0].compare(db) == 0)
			{
				long long dbSize = strtoll(vt_tmp[1].c_str(),NULL,10);
				
				string pwd = GetEnvVar("MYSQLPWD");
				success = LimitMySQLSize(ip,"root",pwd,db,db,dbSize);
				break;
			}
		}
		if(!success)
		{
			errInfo.append("revoke or grant failed.");
			WriteParam(category,vt_param,"failed");
		}
		else
		{
			WriteParam(category,vt_param,"success");
		}
		return success;
	}
	else
	{
		pthread_mutex_lock(&mutex);
		int ret = stat("/usr/local/apache_conf/mysql.size",&buf);
		string param;
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
			fclose(fp);
			vt_conf.push_back(param);
		}
		else
		{
			if(vt_conf.size() == 0)
			{
				if(!ReadFile(&vt_conf,"/usr/local/apache_conf/mysql.size"))
				{
					errInfo.append("read /usr/local/apache_conf/mysql.size failed.");
					WriteLog(category,ERROR,"read /usr/local/apache_conf/mysql.size failed");
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
				if(vt_tmp.size() < 2)
				{
					it++;
					continue;
				}
				if(vt_tmp[0].compare(db) == 0)
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
				errInfo.append("write /usr/local/apache_conf/mysql.size failed.");
				WriteLog(category,ERROR,"write /usr/local/apache_conf/mysql.size failed");	
				pthread_mutex_unlock(&mutex);
				return false;
			}
		}
		pthread_mutex_unlock(&mutex);
		WriteParam(category,vt_param,"success");
		return true;
	}
}
