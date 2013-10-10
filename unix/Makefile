SRC_DIR=../src

TARGET_DIR=tmp
TARGET=${TARGET_DIR}/apache_conf
OBJS=${TARGET_DIR}/main.o

.PHONY:all
all:${TARGET_DIR}/libsock.so ${TARGET_DIR}/libftp.so ${TARGET_DIR}/libtools.so ${TARGET_DIR}/libhostOperation.so ${TARGET_DIR}/libconfig.so ${TARGET_DIR}/liblog.so ${TARGET_DIR}/libprocMySQL.so ${TARGET_DIR}/libaction.so ${TARGET_DIR}/libvirtualHost.so ${TARGET} cleanObj

${TARGET_DIR}/liblog.so:${SRC_DIR}/log.cpp
	g++ -w -D__unix -fPIC -shared $? -o $@ -lzlog -lpthread

${TARGET_DIR}/libsock.so:${SRC_DIR}/sock.cpp
	g++ -w -D__unix -fPIC -shared $? -o $@

${TARGET_DIR}/libtools.so:${SRC_DIR}/tools.cpp
	g++ -w -D__unix -fPIC -shared $? -o $@

${TARGET_DIR}/libhostOperation.so:${SRC_DIR}/hostOperation.cpp
	g++ -w -D__unix -fPIC -shared $? -o $@

${TARGET_DIR}/libftp.so:${SRC_DIR}/ftp.cpp
	g++ -w -D__unix -fPIC -shared $? -o $@

${TARGET_DIR}/libconfig.so:${SRC_DIR}/config.cpp
	g++ -w -D__unix -fPIC -shared $? -o $@

${TARGET_DIR}/libvirtualHost.so:${SRC_DIR}/common.cpp
	g++ -w -D__unix -fPIC -shared $? -o $@

${TARGET_DIR}/libaction.so:${SRC_DIR}/action.cpp
	g++ -w -D__unix -fPIC -shared $? -o $@

${TARGET_DIR}/libprocMySQL.so:${SRC_DIR}/procMySQL.cpp
	g++ -w -D__unix -fPIC -shared $? -o $@ -lmysqlclient -L/usr/lib/mysql/

${TARGET}:${OBJS}
	g++ $? -lhostOperation -lsock -ltools -lftp -lconfig -lpthread -lvirtualHost -laction -lprocMySQL -llog -L./tmp/ -o $@

${TARGET_DIR}/main.o:main.cpp
	g++ -w -c $? -o $@

cleanObj:
	rm ${OBJS}

.PHONY:clean

clean:
	rm ${TARGET_DIR}/*.so
	rm ${TARGET}

.PHONY:install

install:
	-mkdir /usr/local/apache_conf
	cp ${TARGET_DIR}/*.so /usr/lib
	cp ${TARGET_DIR}/libzlog.so.1.1 /usr/lib
	/sbin/ldconfig
	cp ${TARGET_DIR}/apache_conf /usr/local/apache_conf
	cp ${TARGET_DIR}/zlog.conf /usr/local/apache_conf
	cp ${TARGET_DIR}/apache_conf.conf /usr/local/apache_conf

.PHONY:uninstall

uninstall:
	rm -f /usr/lib/libhostOperation.so /usr/lib/libsock.so /usr/lib/libftp.so /usr/lib/libtools.so /usr/lib/libconfig.so /usr/lib/liblog.so /usr/lib/libvirtualHost.so /usr/lib/libaction.so /usr/lib/libaction.so
	rm -rf /usr/local/apache_conf
