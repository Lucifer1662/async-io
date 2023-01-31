#include "raw_socket.h"
#include "os_socket.h"

std::optional<RawSocket> RawSocket::create() {
    OS::SOCKET fd;
    if (OS::INVALID_SOCKET == (fd = OS::create_tcp_socket())) {
        OS::CLOSESOCK(fd);
        return {};
    }

    if (!OS::non_blocking_socket(fd)) {
        OS::CLOSESOCK(fd);
        return {};
    }

    return RawSocket(fd);
}

void RawSocket::read_available() { read_operation.available(); }

void RawSocket::write_available() { write_operation.available(); }

#include <iostream>

AsyncTask<> RawSocket::connect(const IPAddress &address) {
    bool failed = OS::connect(fd, address.ip_to_string().c_str(), address.port);

    int w;
    std::cout << (failed ? "is fine" : "is bad");

    co_await write_operation.wait();

    co_return;
}

void RawSocket::destroy_dependent_coroutines() {
    read_operation.destroy_corourtine();
    write_operation.destroy_corourtine();
}

bool RawListeningSocket::start_listening(int port) {
    this->port = port;
    return OS::listen(fd, port);
}

AsyncTask<std::optional<RawSocket>> RawListeningSocket::accept() {
    OS::SOCKET fd_new_connection;
    do {
        fd_new_connection = co_await accept_operation(port);
        OS::non_blocking_socket(fd_new_connection);
        co_yield RawSocket(fd_new_connection);
    } while (fd_new_connection != OS::INVALID_SOCKET);

    co_return {};
}

std::optional<RawListeningSocket> RawListeningSocket::create() {
    OS::SOCKET fd;
    if (OS::INVALID_SOCKET == (fd = OS::create_tcp_socket())) {
        OS::CLOSESOCK(fd);
        return {};
    }

    if (!OS::non_blocking_socket(fd)) {
        OS::CLOSESOCK(fd);
        return {};
    }

    return RawListeningSocket(fd);
}

void RawListeningSocket::accept_available() { accept_operation.available(); }

void RawListeningSocket::destroy_dependent_coroutines() { accept_operation.destroy_corourtine(); }
