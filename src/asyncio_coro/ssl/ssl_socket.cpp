#include "ssl_socket.h"
#include "../IpAddress.h"
#include <openssl/ssl.h>

#include "ssl_context.h"

SSL_Socket::SSL_Socket(SocketContext &context, SSLContext &sslContext, OS::SOCKET socket, SSL *ssl)
    : context(context)
    , sslContext(sslContext)
    , fd(socket)
    , ssl_fd(ssl)
    , read_operation(fd, ssl, context)
    , write_operation(fd, ssl, context) {}

std::optional<SSL_Socket> SSL_Socket::create(SocketContext &context, SSLContext &ssl_context) {
    OS::SOCKET fd;
    if (OS::INVALID_SOCKET == (fd = OS::create_tcp_socket())) {
        OS::CLOSESOCK(fd);
        return {};
    }

    if (!OS::non_blocking_socket(fd)) {
        OS::CLOSESOCK(fd);
        return {};
    }

    SSL *ssl_fd = SSL_new(ssl_context.context());
    if (ssl_fd == nullptr) {
        OS::CLOSESOCK(fd);
        return {};
    }

    SSL_set_fd(ssl_fd, fd);

    return std::make_optional<SSL_Socket>(context, ssl_context, fd, ssl_fd);
}

struct SSLConnectionFailedException : public std::exception {
    std::string message;
    SSLConnectionFailedException(std::string message)
        : message(std::move(message)) {}

    const char *what() const noexcept override { return message.c_str(); };
};

AsyncTask<> SSL_Socket::connect(const IPAddress &address) {
    if (!OS::connect(fd, address.ip_to_string().c_str(), address.port)) {
        throw SSLConnectionFailedException("Connection to " + address.ip_to_string() + " failed when tcp connecting");
    }

    co_await write_operation.wait();

    int ret;
    while ((ret = SSL_connect(ssl_fd)) == -1) {

        switch (SSL_get_error(ssl_fd, ret)) {
        case SSL_ERROR_WANT_READ:
            co_await read_operation.wait();
            break;
        case SSL_ERROR_WANT_WRITE:
            co_await write_operation.wait();
            break;
        default:
            throw SSLConnectionFailedException("Connection to " + address.ip_to_string() +
                                               " failed when ssl connecting");
        }
    }

    co_return;
}
