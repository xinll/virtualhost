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
//#endif // !DEFINEHEADERS

#ifdef __unix
#define CONF_DIR			"/etc/httpd/vhost.d/"
#define CONF_NAME(name)		#name
#define EXENTESION			".conf"
#define CONF_PATH(name)		CONF_DIR CONF_NAME(name) EXENTESION
#define NEWLINE             "\n"

#define BACKUPDIR           "/backup_main/"
#define USER_ROOT           "/var/www/virtual/"
#define USER_SHELL          "/sbin/nologin"
#define SUPE_HOME           "/home/xinll/code/unix/superhome"

#endif

#ifdef WIN32
#define NEWLINE             "\r\n"
#endif

#define CGI_TPL             "php-cgi.sample"
#define PHPINI_TPL          "php.ini.sample"
#define CONF_TPL            "vhost-conf.sample"

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

#ifndef PATH_MAX
#define PATH_MAX 256
#endif

//socket键值对分隔符
#define SPLIT               "\r\n"

//COMTYPE和NFUNC分别为代表动作类型和具体功能的键
#define COMTYPE             "comtype"
#define NFUNC               "nfunc"

//动作类型,对应于comtype
#define STRHOST             "host"
#define STRSQL              "sql"
//虚拟主机下具体功能,对应于nfunc
#define ERRORDOCUMENT       "errorDocument"
#define CREATEHOST          "createHost"
#define REMOVEHOST          "removeHost"
#define FILEPERMISSION      "filePermission"
#define RESTORECONF         "restoreConf"
#define MYSQLBACK           "backupMySQL"
#define MYSQLRESTORE        "restoreMySQL"
#define DELETEDIR           "rmDir"
#define LIMITMYSQLSIZE      "limitMySQL"
#define CHECKMYSQLSIZESTATE "modifyMySQLLimitState"
#define REDIRECT            "redirect"
#define MINE                "mine"
#define DIRECTORYPERMISSION "directoryAccess"
#define COMPRESS            "compress"
#define UNCOMPRESS          "uncompress"
#define DIRECTORYINDEX      "DirectoryIndex"
#define SERVERALIAS         "ServerAlias"
#define PASSWD              "passwd"
#define MANAGER             "manager"

//管理功能
#define EXPIRED             "expired"
#define STOP                "stop"
#define STOP2               "stop2"
#define STOP3               "stop3"
#define OPEN                "open"
#define QUOTA               "quota"
#define STARTFTP            "startftp"
#define STOPFTP             "stopftp"
#define EXPTME              "/var/www/html/exptme"
#define STOPDIR             "/var/www/html/stop"
#define STOPDIR2            "/var/www/html/stop2"
#define STOPDIR3            "/var/www/html/stop3"
#define FTPUSERS            "/etc/ftpusers"
#define DOCUMENTROOT        "DocumentRoot"
#define SIZE                "size"
//开通主机
#define DOMAIN              "domain"
#define MAX_TRANS           "maxtrans"

//修改密码
#define PWD                 "pwd"
//绑定
#define HOSTBIND            "host"
//DirectoryIndex
#define INDEX                "index"

//压缩文件
#define FILENAME            "zipName"
//目录IP访问限制
#define ADDRESS             "ip"
#define DIRECTORY           "directory"

//修改错误页面参数,对应于errorDocument
#define ERRORNMSTR          "errorNum"
#define ERRORPAGE           "errorPage"
#define USERNAME            "ftpName"

//修改脚本权限参数,对应于filePermission
#define PERMISSION          "permission"
#define PERMISSION_ALLOW    1
#define PERMISSION_DENY     0

//增加虚拟主机参数,对应于createHost
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
#define MYSQLBAKNAME        "mySQLBakFileName"
#define FTPDIR              "ftpDir"

#define SUCCESS             "10000:"
#define PARAMEND            ".\r\n"

//日志级别
#define DEBUG               1
#define INFO                2
#define NOTICE              3
#define WARN                4
#define ERROR               5
#define FATAL               6

//限制数据库大小
#define DBNAME              "dbName"
#define MYSQLSIZE           "size"

//301重定向
#define REDIRECTFROM        "from"
#define REDIRECTTO          "to"
#define REWRITEENGINE       "RewriteEngine"
#define ACTION              "action"
#define URL                 "url"

//mime类型
#define MINETYPE            "mineType"
#define MINEPROCESS         "mineMethod"

//获取MySQL数据库大小
#define MYSQLGETSIZE        "getSize"
#endif // !DEFINEHEADERS
