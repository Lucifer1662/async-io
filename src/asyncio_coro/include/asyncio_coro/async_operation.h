#pragma once
#include "async_task.h"
#include "os_socket.h"
#include <iostream>
#include <cstring>
#include <errno.h>

struct SocketContext;
namespace AsyncOperation {

AsyncTask<int> read(OS::SOCKET fd, SocketContext &context, char *data, int n);

}   // namespace AsyncOperation

template <typename AsyncOperation> AsyncTask<> async_stream(char *data, int n, AsyncOperation &operation) {
    while (n != 0) {
        auto num = operation.poll_op(data, n);
        if (num == -1) {
            std::cout << strerror(errno) << std::endl;
            std::cout << errno << std::endl;
            num = 0;
        }
        n -= num;
        data += num;

        if (num < n) {
            co_await operation.wait();
        }
    }

    int w = 0;
}

struct OverflowOperation {
    std::vector<char> overflow;
    size_t overflow_start = 0;

    int operator()(char *&data, int &n);
    void assign(char *data, int n);
};

struct ReadAsyncOperation {
    OS::SOCKET fd;
    OverflowOperation overflow;
    SocketContext &context;

    ReadAsyncOperation(OS::SOCKET fd, SocketContext &context);
    AsyncTask<> operator()(char *data, int n);
    int poll_op(char *data, int n);
    AsyncTask<> wait();
    void assign_overflow(char *data, int n);
};

struct WriteAsyncOperation {
    OS::SOCKET fd;
    SocketContext &context;

    WriteAsyncOperation(OS::SOCKET fd, SocketContext &context);
    int poll_op(const char *data, int n) { return OS::send(fd, data, n); }
    AsyncTask<> operator()(const char *data, int n) { return async_stream((char *)data, n, *this); }
    AsyncTask<> wait();
};

#include <iostream>
#include <errno.h>

struct AcceptAsyncOperation {
    OS::SOCKET fd;
    SocketContext &context;

    AcceptAsyncOperation(OS::SOCKET fd, SocketContext &context);
    OS::SOCKET poll_op(int port) { return OS::accept(fd, port); }
    AsyncTask<> wait();
    AsyncTask<OS::SOCKET> operator()(int port);
};