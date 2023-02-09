#include "asyncio_coro/listening_socket.h"
#include "asyncio_coro/socket_context.h"

ListeningSocket::ListeningSocket(SocketContext &context, OS::SOCKET socket)
    : context(context)
    , socket(socket)
    , accept_operation(socket, context) {}

std::optional<ListeningSocket> ListeningSocket::create(SocketContext &context) {
    OS::SOCKET fd;
    if (OS::INVALID_SOCKET == (fd = OS::create_tcp_socket())) {
        OS::CLOSESOCK(fd);
        return {};
    }

    if (!OS::non_blocking_socket(fd)) {
        OS::CLOSESOCK(fd);
        return {};
    }

    return ListeningSocket(context, fd);
}

AsyncTask<Socket> ListeningSocket::accept() {
    OS::SOCKET fd_new_connection;
    fd_new_connection = co_await accept_operation(port);
    OS::non_blocking_socket(fd_new_connection);
    co_return Socket(context, fd_new_connection);
}

bool ListeningSocket::start_listening(int port) {
    this->port = port;
    return OS::listen(socket, port);
}
