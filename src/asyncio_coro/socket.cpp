#include "socket.h"
#include "socket_context.h"

Socket::Socket(SocketContext &context, RawSocket socket)
    : context(context)
    , RawSocket(socket) {
    context.add_socket(FD(), this);
}

std::optional<Socket> Socket::create(SocketContext &context) {
    auto socket = RawSocket::create();
    if (!socket) {
        return {};
    }
    return std::make_optional<Socket>(context, *socket);
}

std::optional<std::unique_ptr<Socket>> Socket::create_ptr(SocketContext &context) {
    auto socket = RawSocket::create();
    if (!socket) {
        return {};
    }
    return std::make_unique<Socket>(context, *socket);
}

Socket::~Socket() { context.remove_socket(FD()); }
