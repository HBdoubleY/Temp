#ifndef SYSSOCK_H
#define SYSSOCK_H

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>

typedef unsigned int ULONG;

#define INVALID_SOCKET  -1
#define SOCKET_ERROR    -1

#define closesocket close

inline int SocketGetErrNo(int ifd) {
	return errno;
}

inline void SocketSetErrNo(int ifd, int err) {
	errno = 0;
}

inline int ioctlsocket(int fd, int command, unsigned int* arg) {
	return ioctl(fd, command, arg);
}

#endif //SYSSOCK_H
