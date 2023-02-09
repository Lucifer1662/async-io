#pragma once
#include "../async_task.h"
#include "../os_socket.h"
#include <cstring>
#include "../async_operation.h"

struct SocketContext;
typedef struct ssl_st SSL;

struct SSLReadAsyncOperation {
    OS::SOCKET fd;
    SSL *ssl;

    OverflowOperation overflow;
    SocketContext &context;

    SSLReadAsyncOperation(OS::SOCKET fd, SSL *ssl, SocketContext &context);
    AsyncTask<> operator()(char *data, int n);
    int poll_op(char *data, int n);
    AsyncTask<> wait();
    void assign_overflow(char *data, int n);
};

struct SSLWriteAsyncOperation {
    OS::SOCKET fd;
    SocketContext &context;
    SSL *ssl;

    SSLWriteAsyncOperation(OS::SOCKET fd, SSL *ssl, SocketContext &context);
    int poll_op(const char *data, int n);
    AsyncTask<> operator()(const char *data, int n);
    AsyncTask<> wait();
};
