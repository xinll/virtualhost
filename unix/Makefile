SRC_DIR=../src

TARGET_DIR=tmp
TARGET=${TARGET_DIR}/apache_conf
OBJS=${TARGET_DIR}/main.o\
	 ${TARGET_DIR}/mysqlbak.o\
	 ${TARGET_DIR}/common.o\
	 ${TARGET_DIR}/action.o

.PHONY:all
all:${TARGET_DIR}/libsock.so ${TARGET_DIR}/libftp.so ${TARGET_DIR}/libtools.so ${TARGET_DIR}/libhostOperation.so ${TARGET_DIR}/libconfig.so ${TARGET_DIR}/liblog.so ${TARGET} cleanObj

${TARGET_DIR}/liblog.so:${SRC_DIR}/log.cpp
	g++ -g -w -D__unix -fPIC -shared $? -o $@ -lzlog -lpthread

${TARGET_DIR}/libsock.so:${SRC_DIR}/sock.cpp
	g++ -g -w -D__unix -fPIC -shared $? -o $@

${TARGET_DIR}/libtools.so:${SRC_DIR}/tools.cpp
	g++ -g -w -D__unix -fPIC -shared $? -o $@

${TARGET_DIR}/libhostOperation.so:${SRC_DIR}/hostOperation.cpp
	g++ -g -w -D__unix -fPIC -shared $? -o $@ -I/usr/local/log4cxx/include

${TARGET_DIR}/libftp.so:${SRC_DIR}/ftp.cpp
	g++ -g -w -D__unix -fPIC -shared $? -o $@

${TARGET_DIR}/libconfig.so:${SRC_DIR}/config.cpp
	g++ -g -w -D__unix -fPIC -shared $? -o $@

${TARGET}:${OBJS}
	g++ -g $? -lhostOperation -lsock -ltools -lftp -lconfig -lpthread -lmysqlclient -llog -L./tmp/ -L/usr/lib/mysql/ -o $@

${TARGET_DIR}/main.o:main.cpp
	g++ -g -w -c $? -o $@

${TARGET_DIR}/mysqlbak.o:${SRC_DIR}/procMySQL.cpp
	g++ -g -w -c $? -o $@

${TARGET_DIR}/common.o:${SRC_DIR}/common.cpp
	g++ -g -w -c $? -o $@

${TARGET_DIR}/action.o:${SRC_DIR}/action.cpp
	g++ -g -w -c $? -o $@

cleanObj:
	rm ${OBJS}

.PHONY:clean

clean:
	rm ${TARGET_DIR}/*.so
	rm ${TARGET}

.PHONY:install

install:
	mkdir /usr/local/apache_conf

	cp ${TARGET_DIR}/*.so /usr/lib
	cp ${TARGET_DIR}/libzlog.so.1.1 /usr/lib
	/sbin/ldconfig
	cp ${TARGET_DIR}/apache_conf /usr/local/apache_conf
	cp ${TARGET_DIR}/zlog.conf /usr/local/apache_conf
	cp ${TARGET_DIR}/apache_conf.conf /usr/local/apache_conf

.PHONY:uninstall

uninstall:
	rm -f /usr/lib/libhostOperation.so /usr/lib/libsock.so /usr/lib/libftp.so /usr/lib/libtools.so /usr/lib/libconfig.so /usr/lib/liblog.so
	rm -rf /usr/local/apache_conf
