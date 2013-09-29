SRC_DIR=../src

TARGET_DIR=tmp
TARGET=${TARGET_DIR}/apache_conf
OBJS=${TARGET_DIR}/main.o\
	 ${TARGET_DIR}/mysqlbak.o\
	 ${TARGET_DIR}/common.o\
	 ${TARGET_DIR}/action.o

.PHONY:all
all:${TARGET_DIR}/libchangePermission.so ${TARGET_DIR}/libsock.so ${TARGET_DIR}/libftp.so ${TARGET_DIR}/libtools.so ${TARGET_DIR}/libhostOperation.so ${TARGET_DIR}/libconfig.so ${TARGET} cleanObj

${TARGET_DIR}/libchangePermission.so:${SRC_DIR}/changePermission.cpp
	g++ -g -w -D__unix -fPIC -shared $? -o $@

${TARGET_DIR}/libsock.so:${SRC_DIR}/sock.cpp
	g++ -g -w -D__unix -fPIC -shared $? -o $@

${TARGET_DIR}/libtools.so:${SRC_DIR}/tools.cpp
	g++ -g -w -D__unix -fPIC -shared $? -o $@

${TARGET_DIR}/libhostOperation.so:${SRC_DIR}/hostOperation.cpp
	g++ -g -w -D__unix -fPIC -shared $? -o $@

${TARGET_DIR}/libftp.so:${SRC_DIR}/ftp.cpp
	g++ -g -w -D__unix -fPIC -shared $? -o $@

${TARGET_DIR}/libconfig.so:${SRC_DIR}/config.cpp
	g++ -g -w -D__unix -fPIC -shared $? -o $@

${TARGET}:${OBJS}
	g++ -g $? -lchangePermission -lhostOperation -lsock -ltools -lftp -lconfig -lpthread -L./tmp/ -o $@

${TARGET_DIR}/main.o:main.cpp
	g++ -g -c $? -o $@

${TARGET_DIR}/mysqlbak.o:${SRC_DIR}/procMySQL.cpp
	g++ -g -c $? -o $@

${TARGET_DIR}/common.o:${SRC_DIR}/common.cpp
	g++ -g -c $? -o $@

${TARGET_DIR}/action.o:${SRC_DIR}/action.cpp
	g++ -g -c $? -o $@

cleanObj:
	rm ${OBJS}

.PHONY:clean

clean:
	rm ${TARGET_DIR}/*

.PHONY:install

install:
	cp ${TARGET_DIR}/*.so /usr/lib
	ldconfig
	cp ${TARGET_DIR}/apache_conf /usr/local/sbin
	cp apache_conf.conf /etc/

.PHONY:uninstall

uninstall:
	rm /usr/lib/libsock.so /usr/lib/libserverError.so /usr/lib/libchangePermission.so /usr/lib/libftp.so /usr/lib/libtools.so /usr/lib/libconfig.so /usr/lib/libhostOperation.so
	ldconfig
	rm /usr/local/sbin/apache_conf
	rm /etc/apache_conf.conf

	
