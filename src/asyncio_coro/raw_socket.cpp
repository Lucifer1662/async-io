#include "raw_socket.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>

std::optional<RawSocket> RawSocket::create() {
    SOCKET fd;
    if (INVALID_SOCKET == (fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))) {
        CLOSESOCK(fd);
        return {};
    }

    ULONG uNonBlockingMode = 1;
    if (SOCKET_ERROR == ioctlsocket(fd, FIONBIO, &uNonBlockingMode)) {
        CLOSESOCK(fd);
        return {};
    }

    return RawSocket(fd);
}

void RawSocket::read_available() { read_operation.available(); }

void RawSocket::write_available() { write_operation.available(); }

AsyncTask<> RawSocket::connect(const IPAddress &address) {
    auto ip_string = address.ip_to_string();

    sockaddr_in windows_address;
    windows_address.sin_family = AF_INET;
    windows_address.sin_addr.s_addr = inet_addr(ip_string.c_str());
    windows_address.sin_port = htons(address.port);

    auto res = ::connect(fd, (SOCKADDR *)&windows_address, sizeof(windows_address));
    if (res == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAECONNREFUSED) {
            printf("Connection refused\n");
        }
    }

    co_await write_operation.wait();

    co_return;
}

bool RawSocket::start_listening(int port) {

    SOCKADDR_STORAGE addr = {0};
    addr.ss_family = AF_INET;
    INETADDR_SETANY((SOCKADDR *)&addr);
    SS_PORT((SOCKADDR *)&addr) = htons(port);

    auto err = bind(fd, (SOCKADDR *)&addr, sizeof(addr));
    if (SOCKET_ERROR == err) {
        printf("Failed to bind %d\n", err);
        return false;
    }

    if (SOCKET_ERROR == listen(fd, 1)) {
        return false;
    }
}

AsyncTask<std::optional<RawSocket>> RawSocket::accept() {
    SOCKET fd_new_connection;
    do {
        co_await read_operation.wait();
        fd_new_connection = ::accept(fd, NULL, NULL);
        co_yield RawSocket(fd_new_connection);
    } while (fd_new_connection != INVALID_SOCKET);

    co_return {};
}
