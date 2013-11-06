#include "sock.h"
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<arpa/inet.h>
#include<syslog.h>
 ServerSocket* ServerSocket::_serverSock = NULL;

 ServerSocket::~ServerSocket()
{
#ifdef WIN32
		WSACleanup();
#endif // WIN32
		closeSocket();
}

bool ServerSocket::createSocket()
{
#ifdef __unix
	s = socket(PF_INET,SOCK_STREAM,0);
	int one = 1;
	setsockopt(s,SOL_SOCKET,SO_REUSEADDR,&one,sizeof(one));
#endif
#ifdef WIN32
	s = socket(AF_INET,SOCK_STREAM,0);
#endif
	return ISVALIDSOCKET(s);
}

bool ServerSocket::bindSocket(short port)
{
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family = AF_INET;
#ifdef WIN32
	addr.sin_addr.S_un.S_addr = INADDR_ANY;
#endif
#ifdef __unix
	addr.sin_addr.s_addr = INADDR_ANY;
#endif // WIN32

	
	addr.sin_port = htons(port);
	
	int ret = bind(s,(struct  sockaddr*)&addr,sizeof(struct sockaddr));
	return ret == 0;
}

bool ServerSocket::listenSocket()
{
	return listen(s,5) == 0;
}

void ServerSocket::closeSocket()
{
	if(ISVALIDSOCKET(s))
		MC_CLOSESOCKET(s);
}

MC_SOCKET ServerSocket::acceptSocket(struct sockaddr* clientAddr)
{
	socklen_t len = sizeof(struct sockaddr);
	MC_SOCKET tmp = accept(s,clientAddr,&len);
	return tmp;
}


ClientSocket::ClientSocket(MC_SOCKET s)
{
	_s = s;
}
ClientSocket::~ClientSocket()
{
	MC_CLOSESOCKET(_s);
	syslog(LOG_INFO,"CLOSE--------------------");
}
int ClientSocket::ReadNetData(char *buf,int size)
{
	return recv(_s,buf,size,0);
}
bool ClientSocket::SendSuccessInfo()
{
	int ret = SendNetData(SUCCESS);
	return ret == strlen(SUCCESS);
}
bool ClientSocket::SendErrorInfo(const char *err)
{
	int ret = SendNetData(err);
	return ret == strlen(err);
}

int ClientSocket::SendNetData(const char *data)
{
	return send(_s,data,strlen(data),0);
}

char* ClientSocket::GetPeerName()
{
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);
	getpeername(_s,(struct sockaddr*)&addr,&len);
	return inet_ntoa(addr.sin_addr);
}
