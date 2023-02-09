#include "asyncio_coro/ssl/async_operation.h"
#include "asyncio_coro/ssl/ssl_context.h"
#include "asyncio_coro/socket_context.h"
#include <openssl/ssl.h>

SSLReadAsyncOperation::SSLReadAsyncOperation(OS::SOCKET fd, SSL *ssl, SocketContext &context)
    : fd(fd)
    , ssl(ssl)
    , context(context) {}

int SSLReadAsyncOperation::poll_op(char *data, int n) {
    int amount = overflow(data, n);

    if (n == 0) {
        return amount;
    }

    auto ret = SSL_read(ssl, data, n);
    if (ret == -1)
        ret = 0;

    return amount + ret;
}

AsyncTask<> SSLReadAsyncOperation::wait() { return context.wait_for_read(fd); }

AsyncTask<> SSLReadAsyncOperation::operator()(char *data, int n) { return async_stream(data, n, *this); }

void SSLReadAsyncOperation::assign_overflow(char *data, int n) { overflow.assign(data, n); }

AsyncTask<> SSLWriteAsyncOperation::wait() { return context.wait_for_write(fd); }

SSLWriteAsyncOperation::SSLWriteAsyncOperation(OS::SOCKET fd, SSL *ssl, SocketContext &context)
    : fd(fd)
    , ssl(ssl)
    , context(context) {}

int SSLWriteAsyncOperation::poll_op(const char *data, int n) { return SSL_write(ssl, data, n); }
AsyncTask<> SSLWriteAsyncOperation::operator()(const char *data, int n) { return async_stream((char *)data, n, *this); }
