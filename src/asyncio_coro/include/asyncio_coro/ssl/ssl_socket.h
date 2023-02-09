#pragma once
#include "../os_socket.h"
#include "async_operation.h"
#include <optional>
#include <memory>
#include "../async_task.h"

class SocketContext;
class SSLContext;
class IPAddress;
typedef struct ssl_st SSL;

class SSL_Socket {
    SocketContext &context;
    SSLContext &sslContext;
    OS::SOCKET fd;
    SSL *ssl_fd;

  public:
    SSLReadAsyncOperation read_operation;
    SSLWriteAsyncOperation write_operation;

    SSL_Socket(SocketContext &context, SSLContext &sslContext, OS::SOCKET fd, SSL *ssl);
    SSL_Socket(const SSL_Socket &) = delete;
    SSL_Socket(SSL_Socket &&) = default;

    SSL_Socket &operator=(SSL_Socket &&) = default;

    static std::optional<SSL_Socket> create(SocketContext &context, SSLContext &ssl_context);

    AsyncTask<> connect(const IPAddress &address);
};
