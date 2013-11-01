SRC_DIR=../src

TARGET_DIR=tmp
TARGET=${TARGET_DIR}/apache_conf
OBJS=${TARGET_DIR}/main.o

.PHONY:all
ifeq ($(FLAGS),-DWITH_MYSQL)
all:${TARGET_DIR}/libsock.so ${TARGET_DIR}/libsqlOperation.so ${TARGET_DIR}/libftp.so ${TARGET_DIR}/libtools.so ${TARGET_DIR}/libhostOperation.so ${TARGET_DIR}/libconfig.so ${TARGET_DIR}/liblog.so ${TARGET_DIR}/libprocMySQL.so ${TARGET_DIR}/libaction.so ${TARGET_DIR}/libvirtualHost.so ${TARGET} cleanObj
else
all:${TARGET_DIR}/libsock.so ${TARGET_DIR}/libftp.so ${TARGET_DIR}/libtools.so ${TARGET_DIR}/libhostOperation.so ${TARGET_DIR}/libconfig.so ${TARGET_DIR}/liblog.so ${TARGET_DIR}/libaction.so ${TARGET_DIR}/libvirtualHost.so ${TARGET} cleanObj
endif

${TARGET_DIR}/liblog.so:${SRC_DIR}/log.cpp
	g++ -w -g -D__unix -fPIC -shared $? -lzlog -L. -o $@

ifeq ($(FLAGS),-DWITH_MYSQL)
${TARGET_DIR}/libsqlOperation.so:${SRC_DIR}/sqlOperation.cpp
	g++ -w -g -D__unix -fPIC -shared $? -o $@
endif

${TARGET_DIR}/libsock.so:${SRC_DIR}/sock.cpp
	g++ -w -g -D__unix -fPIC -shared $? -o $@

${TARGET_DIR}/libtools.so:${SRC_DIR}/tools.cpp
	g++ -w -g -D__unix -fPIC -shared $? -o $@

${TARGET_DIR}/libhostOperation.so:${SRC_DIR}/hostOperation.cpp
	g++ -w -g -D__unix -fPIC -shared $? -o $@

${TARGET_DIR}/libftp.so:${SRC_DIR}/ftp.cpp
	g++ -w -g -D__unix -fPIC -shared $? -o $@

${TARGET_DIR}/libconfig.so:${SRC_DIR}/config.cpp
	g++ -w -g -D__unix -fPIC -shared $? -o $@

${TARGET_DIR}/libvirtualHost.so:${SRC_DIR}/common.cpp
	g++ -w -D__unix -fPIC -shared $? -o $@

${TARGET_DIR}/libaction.so:${SRC_DIR}/action.cpp
	g++ -w -g -D__unix -fPIC -shared $? -o $@

ifeq ($(FLAGS),-DWITH_MYSQL)
${TARGET_DIR}/libprocMySQL.so:${SRC_DIR}/procMySQL.cpp
	g++ -w -g -D__unix -fPIC -shared $? -o $@ -lmysqlclient -L/usr/lib/mysql/
endif

ifeq ($(FLAGS),-DWITH_MYSQL)
${TARGET}:${OBJS}
	g++ $? -g -lhostOperation -lsock -ltools -lftp -lconfig -lpthread -lvirtualHost -laction -lsqlOperation -lprocMySQL -llog -L./tmp/ -o $@ 
else
${TARGET}:${OBJS}
	g++ $? -g -lhostOperation -lsock -ltools -lftp -lconfig -lpthread -lvirtualHost -laction -llog -L./tmp/ -o $@ 
endif

${TARGET_DIR}/main.o:main.cpp
	g++ -w -g $(FLAGS) -c $? -o $@

cleanObj:
	rm ${OBJS}

.PHONY:clean

clean:
	rm ${TARGET_DIR}/*.so
	rm ${TARGET}

.PHONY:install

install:
	-mkdir /usr/local/apache_conf
	-mkdir /usr/local/apache_conf/log
	cp ${TARGET_DIR}/*.so /usr/lib
	cp libzlog.so.1.1 /usr/lib/
	/sbin/ldconfig
	cp ${TARGET_DIR}/apache_conf /usr/local/apache_conf
	cp ${TARGET_DIR}/zlog.conf /usr/local/apache_conf
	cp ${TARGET_DIR}/apache_conf.conf /usr/local/apache_conf
.PHONY:uninstall

uninstall:
	rm -f /usr/lib/libhostOperation.so /usr/lib/libsock.so /usr/lib/libftp.so /usr/lib/libtools.so /usr/lib/libconfig.so /usr/lib/liblog.so /usr/lib/libvirtualHost.so /usr/lib/libaction.so /usr/lib/libaction.so
	-rm -rf /usr/local/apache_conf
	rm -rf /usr/lib/libzlog.so.1.1
