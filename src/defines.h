#ifndef DEFINEHEADERS
#define DEFINEHEADERS
#ifdef WIN32
#include <WinSock2.h>
typedef	SOCKET				MC_SOCKET;
#define MC_CLOSESOCKET(s)	closesocket(s)
#define ISVALIDSOCKET(s)	s != INVALID_SOCKET
#endif

#ifdef __unix
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
typedef	int					MC_SOCKET;
#define MC_CLOSESOCKET(s)	close(s)
#define ISVALIDSOCKET(s)	s < 0
#endif // __unix
#endif // !DEFINEHEADERS

#ifdef __unix
#define CONF_DIR			"/etc/httpd/vhost.d/"
#define CONF_NAME(name)		#name
#define EXENTESION			".conf"
#define CONF_PATH(name)		CONF_DIR CONF_NAME(name) EXENTESION
#define NEWLINE             "\n"
#define PERMISSION_ALLOW    1
#define PERMISSION_DENY     0
#endif

#ifdef WIN32
#define NEWLINE             "\r\n"
#endif

//ErrorDocumnet 错误页面
#define ERROR404            404
#define ERROR403            403
#define ERROR500            500
#define ERRORHEAD           "ErrorDocument"
#define FILENODE            "Files"
#define ALLOW               "allow"
#define DENY                "deny"
#define ORDER               "Order"

//默认socket接收缓冲区大小
#define BUF_LENGTH          10240
//socket键值对分隔符
#define SPLIT               "\r\n"

//COMTYPE和NFUNC分别为代表动作类型和具体功能的键
#define COMTYPE             "comtype"
#define NFUNC               "nfunc"

//动作类型
#define STRHOST             "host"

//虚拟主机下具体功能
#define ERRORDOCUMENT       "errorDocument"
#define CREATEHOST          "createHost"
#define FILEPERMISSION      "filePermission"
#define RESTORECONF         "restoreConf"
#define MYSQLBACK           "backupMySQL"
#define MYSQLRESTORE        "restoreMySQL"
#define DELETEDIR           "rmDir"
#define LIMITMYSQLSIZE      "limitMySQL"
#define CHECKMYSQLSIZESTATE "modifyMySQLLimitState"

//修改错误页面参数
#define ERRORNMSTR          "errorNum"
#define ERRORPAGE           "errorPage"
#define USERNAME            "ftpName"

//修改脚本权限参数
#define DIRECTORY           "directory"
#define PERMISSION          "permission"
#define PERMISSION_FILE     "file"

//增加虚拟主机参数
#define DEFAULTDOMAIN       "defaultDomain"
#define MAXTRANS            "maxTrans"
#define MAXCONN             "maxConn"

//MySQL备份与恢复参数
#define FTPUSER             "ftpUser"
#define FTPPW               "ftpPw"
#define MYSQLUSER           "mySQLUser"
#define MYSQLPW             "mySQLPw"
#define FTPSERVER           "ftpServer"
#define FTPPORT             "ftpPort"
#define MYSQLSERVER         "mySQLServer"
#define MYSQLBAKNAME        "mySQLBakFileName"
#define MYSQLBASE           "mySQLDataBase"
#define FTPDIR              "ftpDir"

#define USER_ROOT           "/var/www/virtual/"
#define USER_SHELL          "/sbin/nologin"
#define CONF_TPL            "vhost-conf.sample"
#define CGI_TPL             "php-cgi.sample"
#define SUPE_HOME           "/home/xinll/code/unix/superhome"
#define PHPINI_TPL          "php.ini.sample"

#define SUCCESS             "10000:"
#define PARAMEND            ".\r\n"


#define DEBUG               1
#define INFO                2
#define NOTICE              3
#define WARN                4
#define ERROR               5
#define FATAL               6

#define MYSQLADDR           "MySQLServer"
#define DBNAME              "dbName"
#define MYSQLSIZE           "size"
