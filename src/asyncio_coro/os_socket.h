#pragma once
#include <vector>
#include "socket_context_handler.h"

#ifdef WIN32
#include <WinSock2.h>
#else
#include <sys/socket.h>
#include <unistd.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h>

#endif
namespace OS {

#ifdef WIN32

#else
const int INVALID_SOCKET = -1;
using SOCKET = int;

#endif

int read(SOCKET fd, char *data, int n);
int send(SOCKET fd, const char *data, int n);

void sleep(int ms);

void CLOSESOCK(SOCKET &s);

bool non_blocking_socket(SOCKET fd);
SOCKET create_tcp_socket();

bool connect(SOCKET fd, const char *address, int port);
bool listen(SOCKET server_fd, int port);
SOCKET accept(SOCKET fd, int port);

}   // namespace OS