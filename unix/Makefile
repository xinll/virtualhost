SRC_DIR=../src

TARGET_DIR=tmp
TARGET=${TARGET_DIR}/apache_conf
OBJS=${TARGET_DIR}/main.o

ACTIONDEPS=${SRC_DIR}/redirect.cpp ${SRC_DIR}/mime.cpp ${SRC_DIR}/filePermission.cpp ${SRC_DIR}/directoryAccess.cpp ${SRC_DIR}/errorDocument.cpp ${SRC_DIR}/deleteRootDirectory.cpp ${SRC_DIR}/compress.cpp ${SRC_DIR}/directoryIndex.cpp ${SRC_DIR}/hostBind.cpp ${SRC_DIR}/passwd.cpp ${SRC_DIR}/vhost.cpp ${SRC_DIR}/manager.cpp

ifeq ($(FLAGS),-DWITH_MYSQL)
LIBS=liblog.so libsock.so libprocMySQL.so libsqlOperation.so libftp.so libtools.so libconfig.so libvirtualHost.so libaction.so libhostOperation.so libthreadpool.so
else
LIBS=libsock.so libftp.so libtools.so libconfig.so liblog.so libvirtualHost.so libaction.so libhostOperation.so libthreadpool.so
endif

.PHONY:all
ifeq ($(FLAGS),-DWITH_MYSQL)
all:${LIBS} ${TARGET} cleanObj
else
all:${LIBS} ${TARGET} cleanObj
endif

liblog.so:${SRC_DIR}/log.cpp
	g++ -w -g -D__unix -fPIC -shared $? -lzlog -L. -o $@

libsock.so:${SRC_DIR}/sock.cpp
	g++ -w -g -D__unix -fPIC -shared $? -o $@

libtools.so:${SRC_DIR}/tools.cpp
	g++ -w -g -D__unix -fPIC -shared $? -o $@

libftp.so:${SRC_DIR}/ftp.cpp
	g++ -w -g -D__unix -fPIC -shared $? -o $@

libconfig.so:${SRC_DIR}/config.cpp
	g++ -w -g -D__unix -fPIC -shared $? -o $@

ifeq ($(FLAGS),-DWITH_MYSQL)
libsqlOperation.so:${SRC_DIR}/sqlOperation.cpp
	g++ -w -g -D__unix -fPIC -shared $? -o $@

libprocMySQL.so:${SRC_DIR}/procMySQL.cpp
	g++ -w -g -D__unix -fPIC -shared $? -o $@ -lmysqlclient -L/usr/lib/mysql/
endif

libvirtualHost.so:${SRC_DIR}/common.cpp
	g++ -w -D__unix -fPIC -shared $? -o $@

libaction.so:${ACTIONDEPS}
	g++ -w -g -D__unix -fPIC -shared $? -o $@

libhostOperation.so:${SRC_DIR}/hostOperation.cpp ${SRC_DIR}/action.h
	g++ -w -g -D__unix -fPIC -shared $? -o $@

libthreadpool.so:${SRC_DIR}/threadpool.cpp
	g++ -w -D__unix -fPIC -shared $? -o $@ -lpthread

ifeq ($(FLAGS),-DWITH_MYSQL)
${TARGET}:${OBJS}
	g++ $? -g ${LIBS} -L. -o $@ 
else
${TARGET}:${OBJS}
	g++ $? -g ${LIBS} -L. -o $@ 
endif

${TARGET_DIR}/main.o:main.cpp
	g++ -w -g $(FLAGS) -c $? -o $@

cleanObj:
	cp ${LIBS} tmp/
	rm ${OBJS}
	rm ${LIBS}

.PHONY:clean

clean:
	rm ${TARGET_DIR}/*.so
	rm ${TARGET}

.PHONY:install

OLDLIBS=/usr/lib/libftp.so /usr/lib/libtools.so /usr/lib/libconfig.so /usr/lib/libsock.so /usr/lib/libvirtualHost.so /usr/lib/libaction.so /usr/lib/libhostOperation.so /usr/lib/libsqlOperation.so /usr/lib/libprocMySQL.so /usr/lib/libthreadpool.so /usr/lib/liblog.so /usr/lib/libzlog.so.1.1

install:
	-rm -f ${OLDLIBS}
	-mkdir -p /usr/local/apache_conf
	-mkdir -p /usr/local/apache_conf/log
	-mkdir -p /usr/local/apache_conf/lib
	-mkdir -p /usr/local/apache_conf/cfg
	-mkdir -p /usr/local/apache_conf/bin
	-mkdir -p /usr/local/apache_conf/installfiles
	-mkdir -p /usr/local/apache_conf/tpl
	cp ${TARGET_DIR}/*.so /usr/local/apache_conf/lib
	cp ${TARGET_DIR}/libzlog.so.1.1 /usr/local/apache_conf/lib
	cp ${TARGET_DIR}/apache_lib.conf /etc/ld.so.conf.d/
	/sbin/ldconfig
	cp ${TARGET_DIR}/apache_conf /usr/local/apache_conf/bin
	cp ${TARGET_DIR}/zlog.conf /usr/local/apache_conf/cfg
	cp ${TARGET_DIR}/maxtrans.conf /usr/local/apache_conf/cfg
	cp ${TARGET_DIR}/apache_conf.conf /usr/local/apache_conf/cfg
	cp -a ${TARGET_DIR}/superhome /usr/local/apache_conf/installfiles
	cp ${TARGET_DIR}/shopex2.zip /usr/local/apache_conf/installfiles
	cp ${TARGET_DIR}/*.sample /usr/local/apache_conf/tpl
.PHONY:uninstall

uninstall:
	-rm /etc/ld.so.conf.d/apache_lib.conf
	/sbin/ldconfig
	-rm -rf /usr/local/apache_conf
