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

using namespace std;

//全局变量，以后考虑怎么修改，尽量不要使用全局
pthread_mutex_t mutex;  //互斥锁
string dirPath;
zlog_category_t *mainlog;

void* ProcSocket(void *arg);

int main(int argc,char **argv)
{
	char err[256];
	pid_t pid = fork();
	if(pid < 0)
	{
		exit(EXIT_FAILURE);
	}
	if(pid > 0)
	{
		exit(EXIT_FAILURE);
	}
	pid_t sid = setsid();
	if(sid < 0)
	{
		perror("setsid:");
		exit(EXIT_FAILURE);
	}
	if(chdir("/") < 0)
	{
		perror("chdir:");
		exit(EXIT_FAILURE);
	}
	umask(0);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	
	//初始化python环境
/*	if(!InitPythonEnv())
	{
		syslog(LOG_EMERG,"the python environment is not configured!!!");
		exit(EXIT_FAILURE);
	}
	*/

	//初始化log环境
	InitLog();
	mainlog = GetCategory("main");

	pthread_mutex_init(&mutex,NULL);

	/*加载配置文件，读取数据*/
	Config config;
	config.LoadConfigFile();
	//读取监听端口
	string tmp = config.GetValue("LISTENPORT");
	int port = 10000;
	if(!tmp.empty())
	{
		port = atoi(tmp.c_str());
	}
	//读取VirtualHost配置文件所在目录
	tmp = config.GetValue("CONF_DIR");
	if(!tmp.empty())
	{
		dirPath = tmp;
	}
	else
		dirPath = "/etc/httpd/vhost.d/";
	/*读取配置文件参数结束*/

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
		while(pStr = strstr(tmp,PARAMEND))
		{
			vector<pair<string,string> > vt_param;
			if(!ProcParam(tmp,vt_param))
			{
				string t(tmp,pStr - tmp);
				errInfo.append("failed to reslove the param. ");
				sprintf(err,"the param %s is not right from %s",t.c_str(),clientAddr);
				WriteLog(mainlog,ERROR,err);
				tmp = pStr + strlen(PARAMEND);
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
						errInfo.append("|");
					}
				}
			}
		}
		if(errInfo.empty())
		{
			errInfo = SUCCESS;
			sprintf(err,"success from %s",clientAddr);
			WriteLog(mainlog,INFO,err);
		}
		else
		{
			if(errInfo.c_str()[errInfo.length() - 1] == '|')
				errInfo = errInfo.substr(0,errInfo.length() - 1);
			sprintf(err,"%s from %s",errInfo.c_str(),clientAddr);
			WriteLog(mainlog,ERROR,err);
		}
		acceptSock->SendErrorInfo(errInfo.c_str());
	}
	delete acceptSock;

	pthread_exit(NULL);
}
