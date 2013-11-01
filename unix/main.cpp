#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <utility>
#include <syslog.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <vector>
#include "../src/sock.h"
#include "../src/tools.h"
#include "../src/hostOperation.h"
//#include "../src/callPython.h"
#include "../src/config.h"
#include "../src/log.h"
#include <sys/time.h>
#include <signal.h>
#include "../src/procMySQL.h"
#include <sys/types.h>
#include <sys/wait.h>
#include "../src/sqlOperation.h"

using namespace std;

//全局变量，以后考虑怎么修改，尽量不要使用全局
pthread_mutex_t mutex; 
char *mainlog = "mainLog";

void* ProcSocket(void *arg);
void  TimerAction(int signo);
void  InitSigAction();
void  InitTimer();
void* StartTimer(void *arg);
int   Children();

int main(int argc,char **argv)
{
	int status;

	pid_t pid = fork();
	if(pid < 0)
	{
		exit(EXIT_FAILURE);
	}
	else if(pid == 0)
	{
		pid_t sid = setsid();
		if(sid < 0)
		{
			exit(EXIT_FAILURE);
		}
		if(chdir("/") < 0)
		{
			exit(EXIT_FAILURE);
		}
		umask(0);
		close(STDIN_FILENO);
		close(STDOUT_FILENO);
		close(STDERR_FILENO);
		for(;;)
		{
			pid_t tmp = fork();
			if(tmp < 0)
			{
				exit(EXIT_FAILURE);
			}
			else if(tmp == 0)
			{
				Children();
			}
			else
			{
				waitpid(pid,&status,0);
				if(WIFEXITED(status))
				{
					if(WEXITSTATUS(status) == 0)
						exit(0);
					if(WIFSIGNALED(status))
					{
						switch(WTERMSIG(status))
						{
							case SIGKILL:
								exit(0);
								break;
							default:
							{
								break;
							}
						}
					}
				}
				sleep(100);
				continue;
			}
		}
	}	
	else
		exit(EXIT_FAILURE);
	return 0;
}

int Children()
{
//	signal(SIGTERM,ProcSig);
	signal(SIGINT,SIG_IGN);
	signal(SIGQUIT,SIG_IGN);

	char err[256];
	//初始化python环境
/*	if(!InitPythonEnv())
	{
		syslog(LOG_EMERG,"the python environment is not configured!!!");
		exit(EXIT_FAILURE);
	}
	*/

	//初始化log环境
	InitLog();

	pthread_mutex_init(&mutex,NULL);

	//读取监听端口
	string tmp = GetEnvVar("LISTENPORT");
	int port = 10000;
	if(!tmp.empty())
	{
		port = atoi(tmp.c_str());
	}

	ServerSocket *serverSock = ServerSocket::CreateInstance();
	serverSock->createSocket();
	if(!serverSock->bindSocket((short)port))
	{
		sprintf(err,"can't bind the port %d.",port);
		WriteLog(mainlog,FATAL,err);
		exit(EXIT_FAILURE);
	}
	if(!serverSock->listenSocket())
	{
		sprintf(err,"can't listen on port %d.",port);
		WriteLog(mainlog,FATAL,err);
		exit(EXIT_FAILURE);
	}
	
	WriteLog(mainlog,INFO,"the system start normally!!!");

#ifdef WITH_MYSQL
	pthread_t timer;
	int timer_return = pthread_create(&timer,NULL,StartTimer,NULL);
	if(timer_return != 0)
	{
		WriteLog(mainlog,ERROR,"can't start the timer");
		exit(EXIT_FAILURE);
	}
#endif
	MC_SOCKET sock;
	struct sockaddr_in clientAddr;
	while(1)
	{
		WriteLog(mainlog,INFO,"ready to accpet connection");
		sock = serverSock->acceptSocket((struct sockaddr*)&clientAddr);
		if(sock <= 0)
			break;
		sprintf(err,"accpet from client:%s",inet_ntoa(clientAddr.sin_addr));
		WriteLog(mainlog,INFO,err);

		pthread_t thd;
		int ret = pthread_create(&thd,NULL,ProcSocket,&sock);
		if(ret != 0)
		{
			sprintf(err,"failed to create the thread to proc this socket from %s.",inet_ntoa(clientAddr.sin_addr));
			WriteLog(mainlog,INFO,err);
		}
	}
	delete serverSock;
//	UnInitPythonEnv();
	UnInitLog();
	WriteLog(mainlog,INFO,"the system is ready to exit!!!");
	return 0;
}

void* ProcSocket(void *arg)
{
	MC_SOCKET sock = *(MC_SOCKET*)arg;
	ClientSocket *acceptSock = new ClientSocket(sock);
	char *clientAddr = acceptSock->GetPeerName();
	int total = 0;
	string errInfo = "";
	char err[4096];
	while(1)
	{
		errInfo = "";
		fd_set fds;
		FD_ZERO(&fds);
		struct timeval tv;
		tv.tv_sec = 10;
		tv.tv_usec = 0;
		FD_SET(sock,&fds);
		if(select(sock + 1,&fds,NULL,NULL,&tv) <= 0)
			break;
		char buf[BUF_LENGTH];
		memset(buf,0,BUF_LENGTH);
		int retByte = 0;
		if((retByte = acceptSock->ReadNetData(buf,BUF_LENGTH)) < 0)
		{
			sprintf(err,"read from %s error.",clientAddr);
			WriteLog(mainlog,ERROR,err);
			break;
		}
		else if(retByte == 0)
		{
			sprintf(err,"%s close this connection.",clientAddr);
			WriteLog(mainlog,DEBUG,err);
			break;
		}
		sprintf(err,"Receive Data:%s,length = %d.",buf,retByte);
		WriteLog(mainlog,INFO,err);
		total += retByte;
		char *end = strstr(buf,PARAMEND);
		if(end == NULL || strcmp(buf + strlen(buf) -3,PARAMEND) != 0)
		{
			errInfo.append("the param error.");
			acceptSock->SendErrorInfo(errInfo.c_str());
			sprintf(err,"the param error from %s.",clientAddr);
			WriteLog(mainlog,ERROR,err);
			delete acceptSock;
			pthread_exit(NULL);
		}
		char *pStr = NULL;
		char *tmp = buf;
		vector<string> ret_value;
		while(pStr = strstr(tmp,PARAMEND))
		{
			errInfo = "";
			vector<pair<string,string> > vt_param;
			if(!ProcParam(tmp,vt_param))
			{
				string t(tmp,pStr - tmp);
				errInfo.append("failed to reslove the param. ");
				sprintf(err,"the param %s is not right from %s",t.c_str(),clientAddr);
				WriteLog(mainlog,ERROR,err);
				tmp = pStr + strlen(PARAMEND);
				ret_value.push_back(errInfo);
				continue;
			}
			tmp = pStr + strlen(PARAMEND);
					
			pair<string,string> p = vt_param[0];
			if(p.first.compare(COMTYPE) ==0)
			{
				if(p.second.compare(STRHOST) == 0)
				{
					if(!ProcHost(vt_param,errInfo))
					{
						ret_value.push_back(errInfo);
					}
				}
#ifdef WITH_MYSQL
				else if(p.second.compare(STRSQL) == 0)
				{
					if(!ProcSql(vt_param,errInfo))
					{
						ret_value.push_back(errInfo);
					}
					else
					{
						string tmp = SUCCESS;
						if(vt_param[1].second.compare(MYSQLGETSIZE) == 0)
						{
							tmp.append(errInfo);
							tmp.append(",");
						}
						ret_value.push_back(tmp);
					}
				}
				else
				{
					errInfo = "unknow operation";
				}
#endif
			}
		}
		string response = "";
		int size = ret_value.size();
		for(int i = 0; i < size; i++)
		{
			response.append(ret_value[i]);
			if(i != size -1)
				response.append("|");
		}
		WriteLog(mainlog,INFO,response.c_str());
		acceptSock->SendErrorInfo(response.c_str());
	}
	delete acceptSock;

	pthread_exit(NULL);
}
#ifdef WITH_MYSQL
typedef struct LimitSizeParam{
	string ip;
	string name;
	string pwd;
	string database;
	string ftpName;
	long long size;
}SizeParam;

void TimerAction(int signo)
{
	static vector<SizeParam> vt_param;
	static int               InitTime = 0;
	struct stat buf;
	pthread_mutex_lock(&mutex);
	int ret = stat("/usr/local/apache_conf/mysql.size",&buf);
	pthread_mutex_unlock(&mutex);
	if(ret != 0)
		return;
	if(InitTime == 0)
	{
		InitTime = buf.st_mtime;
	}
	else
	{
		if(InitTime != buf.st_mtime)
		{
			vt_param.clear();
		}
	}
	vector<string> vt_tmp;
	pthread_mutex_lock(&mutex);
	if(!ReadFile(&vt_tmp,"/usr/local/apache_conf/mysql.size"))
	{
		pthread_mutex_unlock(&mutex);
		return;
	}
	InitTime = buf.st_mtime;

	pthread_mutex_unlock(&mutex);
	vector<string>::iterator it;
	vector<string> vt_sizeparam;
	for(it = vt_tmp.begin(); it != vt_tmp.end(); it++)
	{
		vt_sizeparam.clear();
		Split((*it),vt_sizeparam);
		SizeParam p;
		if(vt_sizeparam.size() < 2)
			continue;
		p.ip = "localhost"; //""127.0.0.1";
		p.name = "root";
		p.pwd = GetEnvVar("MYSQLPWD");
		p.ftpName = vt_sizeparam[0];
		p.database = vt_sizeparam[0];
		p.size = strtoll(vt_sizeparam[1].c_str(),NULL,0);
		vt_param.push_back(p);
	}
	vector<SizeParam>::iterator it_doLimit;
	for(it_doLimit = vt_param.begin(); it_doLimit!=vt_param.end(); it_doLimit++)
	{
		LimitMySQLSize((*it_doLimit).ip,(*it_doLimit).name,(*it_doLimit).pwd,(*it_doLimit).ftpName,(*it_doLimit).database,(*it_doLimit).size);	
	}
}

void InitSigAction()
{
	struct sigaction act;
	act.sa_handler = TimerAction;
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	sigaction(SIGPROF,&act,NULL);
}

void InitTimer()
{
	string value = GetEnvVar("TIMERSECONDS");
	if(value.empty())
		value = "43200";
	struct itimerval val;
	val.it_value.tv_sec = atol(value.c_str());
	val.it_value.tv_usec = 0;
	val.it_interval = val.it_value;
	setitimer(ITIMER_PROF,&val,NULL);
}

void* StartTimer(void* arg)
{
	InitSigAction();
	InitTimer();
	while(1)
	{
		sleep(10);
	}
	return NULL;
}
#endif
