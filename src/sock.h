#include "defines.h"

class ServerSocket
{
private:
	MC_SOCKET s;
	static ServerSocket* _serverSock;

private:
	ServerSocket()
	{
		;
	}

public:
	static ServerSocket* CreateInstance()
	{
		if(NULL == _serverSock)
		{
#ifdef WIN32
			WORD wVersion;
			wVersion = MAKEWORD(2,2);
			WSADATA wsaData;
			int ret = WSAStartup(wVersion,&wsaData);
			if(ret != 0)
			{
				return NULL;
			}
#endif // WIN32
			_serverSock = new ServerSocket;
			return _serverSock;
		}
		else
			return _serverSock;
	}

	virtual	~ServerSocket();

	bool createSocket();
	bool bindSocket(short port);
	bool listenSocket();
	void closeSocket();
	MC_SOCKET acceptSocket(struct sockaddr* clientAddr = NULL);
};

class ClientSocket
{
public:
	ClientSocket(MC_SOCKET s);
	~ClientSocket();
	int ReadNetData(char *buf,int size);
	int SendNetData(const char *data);
	bool SendSuccessInfo();
	bool SendErrorInfo(const char *err);
	char *GetPeerName();
private:
	MC_SOCKET _s;
};
