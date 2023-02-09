#include "asyncio_coro/os_socket.h"

#include <iostream>
#include <errno.h>

namespace OS {

#ifdef WIN32

int read(SOCKET fd, char *data, int n) { return recv(fd, data, n, 0); }
int send(SOCKET fd, const char *data, int n) { return ::send(fd, data, n, 0); }

void sleep(int ms) { Sleep(ms); }

void CLOSESOCK(SOCKET &s) {
    if (INVALID_SOCKET != s) {
        closesocket(s);
        s = INVALID_SOCKET;
    }
}

bool non_blocking_socket(SOCKET fd) {
    ULONG uNonBlockingMode = 1;
    return SOCKET_ERROR == ioctlsocket(fd, FIONBIO, &uNonBlockingMode);
}

SOCKET create_tcp_socket() { return socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); }

bool connect(SOCKET fd, const char *address, int port) {
    sockaddr_in windows_address;
    windows_address.sin_family = AF_INET;
    windows_address.sin_addr.s_addr = inet_addr(ip_string.c_str());
    windows_address.sin_port = htons(port);

    auto res = ::connect(fd, (SOCKADDR *)&windows_address, sizeof(windows_address));
    if (res == SOCKET_ERROR) {
        if (WSAGetLastError() == WSAECONNREFUSED) {
            return false;
        }
    }
    return true;
}

SOCKET accept(SOCKET fd, int port) { return accept(fd, NULL, NULL); }

#else

int read(SOCKET fd, char *data, int n) { return recv(fd, data, n, 0); }
int send(SOCKET fd, const char *data, int n) { return ::send(fd, data, n, 0); }

SOCKET create_tcp_socket() { return socket(AF_INET, SOCK_STREAM, 0); }

void CLOSESOCK(SOCKET &s) {
    if (INVALID_SOCKET != s) {
        close(s);
        s = INVALID_SOCKET;
    }
}

bool non_blocking_socket(SOCKET fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return false;
    flags = flags | O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags) == 0;
}

bool connect(SOCKET sock, const char *address, int port) {
    struct sockaddr_in serv_addr;

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    // Convert IPv4 and IPv6 addresses from text to binary
    // form
    if (inet_pton(AF_INET, address, &serv_addr.sin_addr) <= 0) {
        return false;
    }
    int er;
    if ((er = connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr))) < 0) {
        std::cout << "Error: " << errno << std::endl;
        return errno == EINPROGRESS;
    }

    return true;
}

bool listen(SOCKET server_fd, int port) {
    int new_socket, valread;
    struct sockaddr_in address;
    int opt = 1;

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        return false;
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        return false;
    }
    if (::listen(server_fd, 3) < 0) {
        return false;
    }

    return true;
}

SOCKET accept(SOCKET fd, int port) {
    sockaddr address;
    socklen_t addrlen = {0};
    return accept(fd, &address, &addrlen);
}

void sleep(int ms) { ::usleep(ms * 1000); }

#endif

}   // namespace OS