#include "asyncio_coro/socket.h"
#include "asyncio_coro/IpAddress.h"

Socket::Socket(SocketContext &context, OS::SOCKET socket)
    : context(context)
    , fd(socket)
    , read_operation(fd, context)
    , write_operation(fd, context) {}

std::optional<Socket> Socket::create(SocketContext &context) {
    OS::SOCKET fd;
    if (OS::INVALID_SOCKET == (fd = OS::create_tcp_socket())) {
        OS::CLOSESOCK(fd);
        return {};
    }

    if (!OS::non_blocking_socket(fd)) {
        OS::CLOSESOCK(fd);
        return {};
    }

    return std::make_optional<Socket>(context, fd);
}

AsyncTask<> Socket::connect(const IPAddress &address) {
    bool failed = OS::connect(fd, address.ip_to_string().c_str(), address.port);

    co_await write_operation.wait();

    co_return;
}
