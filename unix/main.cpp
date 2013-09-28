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

using namespace std;

//全局变量，以后考虑怎么修改，尽量不要使用全局
vector<string> vt_fileName; //记录已经打开的文件名
pthread_mutex_t mutex;  //互斥锁
string dirPath;

void* ProcSocket(void *arg);

int main(int argc,char **argv)
{
	openlog("apache_conf",LOG_PID,0);
	/*pid_t pid = fork();
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
	*/
	//初始化python环境
/*	if(!InitPythonEnv())
	{
		syslog(LOG_EMERG,"the python environment is not configured!!!");
		exit(EXIT_FAILURE);
	}
	*/
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
		syslog(LOG_EMERG,"can't bind the port %d",port);
		exit(EXIT_FAILURE);
	}
	if(!serverSock->listenSocket())
	{
		syslog(LOG_EMERG,"listen error");
		exit(EXIT_FAILURE);
	}
	
	syslog(LOG_INFO,"the system start normally!!!");
	MC_SOCKET sock;
	struct sockaddr_in clientAddr;
	while(1)
	{
		syslog(LOG_INFO,"ready to accept connection");
		sock = serverSock->acceptSocket((struct sockaddr*)&clientAddr);
		if(sock <= 0)
			break;
		syslog(LOG_INFO,"accpet from client:%s",inet_ntoa(clientAddr.sin_addr));

		pthread_t thd;
		int ret = pthread_create(&thd,NULL,ProcSocket,&sock);
		if(ret != 0)
		{
			syslog(LOG_ERR,"failed to create the thread to proc this socket");
		}
	}
	delete serverSock;
//	UnInitPythonEnv();
	syslog(LOG_INFO,"the system is ready to exit!!!");
	closelog();
	return 0;
}

void* ProcSocket(void *arg)
{
	MC_SOCKET sock = *(MC_SOCKET*)arg;
	ClientSocket *acceptSock = new ClientSocket(sock);
	char *clientAddr = acceptSock->GetPeerName();
	int total = 0;
	string errInfo = ""; 
	while(1)
	{
		fd_set fds;
		FD_ZERO(&fds);
		struct timeval tv;
		tv.tv_sec = 2;
		tv.tv_usec = 0;
		FD_SET(sock,&fds);
		if(select(sock + 1,&fds,NULL,NULL,&tv) <= 0)
			break;
		char buf[BUF_LENGTH];
		memset(buf,0,BUF_LENGTH);
		int retByte = 0;
		if((retByte = acceptSock->ReadNetData(buf,BUF_LENGTH)) < 0)
		{
			//出错
			syslog(LOG_ERR,"read from %s error",clientAddr);
			break;
		}
		else if(retByte == 0)
		{
			syslog(LOG_INFO,"%s close this connection",clientAddr);
			break;
		}
		syslog(LOG_INFO,"接收到数据:%s,长度为：%d",buf,retByte);
		total += retByte;
		char *end = strstr(buf,PARAMEND);
		if(end == NULL || strcmp(buf + strlen(buf) -3,PARAMEND) != 0)
		{
			//continue;
			errInfo.append("参数错误:");
			errInfo.append(buf);
			errInfo.append(PARAMEND);
			syslog(LOG_ERR,errInfo.c_str());
			acceptSock->SendErrorInfo(errInfo.c_str());
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
				//参数解析出错
				string err(tmp,pStr-tmp);
				errInfo.append(err);
				errInfo.append("解析错误");
				errInfo.append(PARAMEND);
				syslog(LOG_ERR,"the param is not right from %s",clientAddr);
				continue;
			}
			char *thisParam = tmp;
			tmp = pStr + strlen(PARAMEND);

			string fileName = "";
			bool exist = false;
			pair<string,string> p = vt_param[0];
			
			for(int i = 0; i < vt_param.size(); i++)
			{
				if(vt_param[i].first.compare(USERNAME) == 0)
				{
					fileName = vt_param[i].second;
					pthread_mutex_lock(&mutex);
					vector<string>::iterator it = vt_fileName.begin();
					for(; it != vt_fileName.end(); it++)
					{
						if((*it).compare(fileName) == 0)
						{
							exist = true;
							break;
						}
					}
					if(exist)
					{
						syslog(LOG_ERR,"the %s from %s is editing",fileName.c_str(),clientAddr);
						pthread_mutex_unlock(&mutex);
						errInfo.append("用户配置文件正在修改,请稍后操作。");
						errInfo.append(PARAMEND);
						acceptSock->SendErrorInfo(errInfo.c_str());
						delete acceptSock; //删除资源
						pthread_exit(NULL);
					}
					else
					{
						vt_fileName.push_back(fileName);
					}
					pthread_mutex_unlock(&mutex);
					break;
				}
			}
			
			if(p.first.compare(COMTYPE) ==0)
			{
				if(p.second.compare(STRHOST) == 0)
				{
					if(!ProcHost(vt_param,errInfo))
					{
						string strParam(thisParam,pStr + strlen(PARAMEND) - thisParam);
						errInfo.append(strParam);
					}
				}
			}
			if(!fileName.empty())
			{
				pthread_mutex_lock(&mutex);
				vector<string>::iterator it = vt_fileName.begin();
				for(; it != vt_fileName.end(); it++)
				{
					if((*it).compare(fileName) == 0)
					{
						vt_fileName.erase(it);
						break;
					}
				}
				pthread_mutex_unlock(&mutex);
			}
		}
	}
	if(errInfo.empty())
	{
		errInfo = SUCCESS;
		errInfo.append(PARAMEND);
		syslog(LOG_INFO,errInfo.c_str());
	}
	else
	{
		syslog(LOG_ERR,errInfo.c_str());
	}
	acceptSock->SendErrorInfo(errInfo.c_str());
	delete acceptSock;
	pthread_exit(NULL);
}
