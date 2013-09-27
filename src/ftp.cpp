#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include "ftp.h"
#include <unistd.h>

using namespace std;

CFTP::CFTP(void)
{
 
}

CFTP::~CFTP(void)
{
 
}

int CFTP::ftp_checkresp(char expresp)
{
    int len = recv(m_sockctrl,m_resp,256,0);
    if(-1 == len) return -1;
    m_resp[len]='\0';
    if(m_resp[0]!=expresp)// return -1;
	{
		fd_set fds;
		FD_ZERO(&fds);
		struct timeval tm;
		tm.tv_sec = 1;
		tm.tv_usec = 0;
		int len = 0;
		while(1)
		{
			FD_ZERO(&fds);
			FD_SET(m_sockctrl,&fds);
			int ret = select(m_sockctrl + 1,&fds,NULL,NULL,&tm);
			if(ret <= 0)
			{
				return -1;
			}
			else
			{
				len = recv(m_sockctrl,m_resp,256,0);
				if(m_resp[0] == expresp)
					return 0;
				else
					continue;
			}
		}
	}
	else
		return 0;
/*	fd_set fds;
	FD_ZERO(&fds);
	struct timeval tm;
	tm.tv_sec = 0;
	tm.tv_usec = 50000;
	int len = 0;
	int tmp = 0;
	while(1)
	{
		FD_ZERO(&fds);
		FD_SET(m_sockctrl,&fds);
		int ret = select(m_sockctrl + 1,&fds,NULL,NULL,&tm);
		if(ret <= 0)
		{
			break;
		}
		else
		{
			len = recv(m_sockctrl,m_resp + tmp,256,0);
			tmp += len;
		}
	}
    //int len = recv(m_sockctrl,m_resp,256,0);
    if(-1 == len) return -1;
    m_resp[len]='\0';
    // puts(m_resp);//应该保存ftp运行日志
    if(m_resp[0]!=expresp) return -1;
    return 0;*/
}

int CFTP::ftp_sendcmd()
{
    int ret = send(m_sockctrl,m_cmd,strlen(m_cmd),0);
    if(-1 == ret)return -1;
    return 0;
}

int CFTP::ftp_connect(const char* ip,short port)
{
    m_sockctrl = socket(AF_INET,SOCK_STREAM,0);
    if(0==m_sockctrl)return -1;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = inet_addr(ip);
    int err = connect(m_sockctrl,(sockaddr*)&addr,sizeof(addr));
    if(err)return -1;
    err = ftp_checkresp('2');
    if(err)return -1;
    return 0;
}

int CFTP::ftp_login(const char* user,const char* pass)
{
    sprintf(m_cmd,"USER %s\r\n",user);
    int err = ftp_sendcmd();
    if(err)return -1;
    err = ftp_checkresp('3');
    if(err)return -1;
    sprintf(m_cmd,"PASS %s\r\n",pass);
	//printf("%s",m_cmd);
    err = ftp_sendcmd();
    if(err)return -1;
    err = ftp_checkresp('2');
    if(err)return -1;
    return 0;
}

int CFTP::ftp_quit()
{
    sprintf(m_cmd,"QUIT\r\n");
    ftp_sendcmd();
    return 0;
}
int CFTP::ftp_cd(const char* dir)
{
    sprintf(m_cmd,"CWD %s\r\n",dir);
    int err = ftp_sendcmd();
    if(err)return -1;
    err = ftp_checkresp('2');
    if(err)return -1;
    return 0;
}

int CFTP::ftp_cdup()
{
    sprintf(m_cmd,"CDUP\r\n");
    int err = ftp_sendcmd();
    if(err)return -1;
    err = ftp_checkresp('2');
    if(err)return -1;
    return 0;
}

int CFTP::ftp_pwd(char* buff)
{
    sprintf(m_cmd,"PWD\r\n");
    int err = ftp_sendcmd();
    if(err)return -1;
    err = ftp_checkresp('2');
    if(err)return -1;
    char* p=m_resp;
    while(*p)
    {
        if(*p++ == '"')
            while(*p!='"')
                *buff++=*p++;
    }
    *buff=0;
   // printf("current work directory is : %s\n",buff);
    return 0;
}
int CFTP::ftp_mkdirSingle(char* dir)
{
	sprintf(m_cmd,"MKD %s\r\n",dir);
    int err = ftp_sendcmd();
    if(err)return -1;
    err = ftp_checkresp('2');
    if(err)return -1;
    return 0;
}
int CFTP::ftp_mkdir(const char* dir)
{
	char path[300];
	int err = ftp_cd("/");
  	if(err)return -1;
	int i,j;
//	printf("strlen(dir):%d",strlen(dir));
	for(i=1,j=0;i<strlen(dir);i++)	//第一个字节是根目录
	{
		path[j++] = dir[i];
		if(dir[i]=='/'){
			path[j++]='\0';
			printf("create :%s\n",path);
			err = ftp_mkdirSingle(path);
			err = ftp_cd(path);
			if(err)return -1;
			j=0;
		}
	}
	path[j++]='\0';
    sprintf(m_cmd,"MKD %s\r\n",path);
    err = ftp_sendcmd();
    if(err)return -1;
    err = ftp_checkresp('2');
    if(err)return -1;
    return 0;
}

int CFTP::ftp_rmdir(char* dir)
{
    sprintf(m_cmd,"RMD %s\r\n",dir);
    int err = ftp_sendcmd();
    if(err)return -1;
    err = ftp_checkresp('2');
    if(err)return -1;
    return 0;
}

int CFTP::ftp_upload(const char* localfile,const char* remotepath,const char* remotefilename)
{

	ftp_mkdir(remotepath);
	int err = ftp_cd(remotepath);
	if(err)return -1;
    ftp_setpasv();
    sprintf(m_cmd,"STOR %s\r\n",remotefilename);
    err = ftp_sendcmd();
    if(err)return -1;
    err = ftp_checkresp('1');
    if(err)return -1;
    FILE* pf = fopen(localfile,"r");
    if(NULL==pf)return -1;
    char sendbuf[256];
    size_t len = 0;
    while((len = fread(sendbuf,1,255,pf))>0)
    {
        err = send(m_sockdata,sendbuf,len,0);
        if(err<0)return -1;
    }
    close(m_sockdata);
    fclose(pf);
    err = ftp_checkresp('2');
    if(err)return -1;
    return 0;
}

int CFTP::ftp_setpasv()
{
    sprintf(m_cmd,"PASV\r\n");
    int err = ftp_sendcmd();
    if(err)return -1;
    err = ftp_checkresp('2');
    if(err)return -1;
    m_sockdata = socket(AF_INET,SOCK_STREAM,0);
    unsigned int v[6];
    union {
        struct sockaddr sa;
        struct sockaddr_in in;
    } sin;
    sscanf(m_resp,"%*[^(](%u,%u,%u,%u,%u,%u",&v[2],&v[3],&v[4],&v[5],&v[0],&v[1]);
	sin.sa.sa_family = AF_INET;
    sin.sa.sa_data[2] = v[2];
    sin.sa.sa_data[3] = v[3];
    sin.sa.sa_data[4] = v[4];
    sin.sa.sa_data[5] = v[5];
    sin.sa.sa_data[0] = v[0];
    sin.sa.sa_data[1] = v[1];

    int on =1;
    if (setsockopt(m_sockdata,SOL_SOCKET,SO_REUSEADDR,
        (const char*) &on,sizeof(on)) == -1)
    {
        perror("setsockopt");
        close(m_sockdata);
        return -1;
    }
    struct linger lng = { 0, 0 };

    if (setsockopt(m_sockdata,SOL_SOCKET,SO_LINGER,
        (const char*) &lng,sizeof(lng)) == -1)
    {
        perror("setsockopt");
        close(m_sockdata);
        return -1;
    }

    err = connect(m_sockdata,(sockaddr*)&sin,sizeof(sin));
    if(err)return -1;
    return 0;
}
int CFTP::ftp_download(const char* localfile,const char* remotefile)
{
/*	sprintf(m_cmd,"TYPE A\r\n",remotefile);
    int err = ftp_sendcmd();
    if(err)return -1;
    err = ftp_checkresp('2');
    if(err)return -1;*/

    ftp_setpasv();
    sprintf(m_cmd,"RETR %s\r\n",remotefile);
    int err = ftp_sendcmd();
    if(err)return -1;
    err = ftp_checkresp('1');
    if(err)return -1;
    FILE* pf = fopen(localfile,"w");
    if(NULL==pf)return -1;
    char recvbuf[256];
    int len = 0;
    while((len = recv(m_sockdata,recvbuf,256,0))>0)
    {
        err = fwrite(recvbuf,len,1,pf);
        if(len<0)return -1;
    }
    close(m_sockdata);
    fclose(pf);
    err = ftp_checkresp('2');
    if(err)return -1;
    return 0;
}

