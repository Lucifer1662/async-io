#pragma once
#include "../buffer/buffer.h"
#include <cstdlib>
#include <functional>
#include <memory>
#include <optional>
#include <queue>
#include <stdio.h>
#include <string>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>

#include "IpAddress.h"
#include "async_operation.h"
#include "base_socket.h"

class Context;

struct Error {};

struct WriteOperation {
    SOCKET fd;
    WriteOperation(SOCKET fd)
        : fd(fd) {}

    int operator()(char *d, int n) { return send(fd, d, n, NULL); }
};

using AsyncWriteOperation = AsyncOperation<WriteOperation>;

struct ReadOperation {
    SOCKET fd;
    ReadOperation(SOCKET fd)
        : fd(fd) {}

    int operator()(char *d, int n) {
        auto ret = recv(fd, d, n, NULL);
        if (WSAGetLastError() == WSAEWOULDBLOCK) {
            return 0;
        }
        return ret;
    }
};

using AsyncReadOperation = AsyncOperation<ReadOperation>;

inline void CLOSESOCK(SOCKET &s) {
    if (INVALID_SOCKET != s) {
        closesocket(s);
        s = INVALID_SOCKET;
    }
}

class WindowsSocket : public BaseSocket<AsyncWriteOperation, AsyncReadOperation> {
    SOCKET fd = INVALID_SOCKET;
    bool connected = false;

  public:
    WindowsSocket(SOCKET fd = INVALID_SOCKET)
        : BaseSocket<AsyncWriteOperation, AsyncReadOperation>({fd}, {fd})
        , fd(fd)
        , connected(fd == INVALID_SOCKET) {}
    WindowsSocket(const WindowsSocket &s) = delete;
    WindowsSocket(WindowsSocket &&s)
        : BaseSocket<AsyncWriteOperation, AsyncReadOperation>(std::move(s))
        , fd(s.fd) {
        s.fd = INVALID_SOCKET;
    };

    auto FD() const { return fd; }

    // on_connection is of signature void(Socket& socket, bool error)
    template <typename Func> bool connect(const IPAddress &address, Func &&on_connection) {

        // socket already connected
        if (fd != INVALID_SOCKET) {
            return false;
        }

        if (INVALID_SOCKET == (fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP))) {
            CLOSESOCK(fd);
            return false;
        }

        readOps.base_operation.fd = fd;
        writeOps.base_operation.fd = fd;

        ULONG uNonBlockingMode = 1;
        if (SOCKET_ERROR == ioctlsocket(fd, FIONBIO, &uNonBlockingMode)) {
            CLOSESOCK(fd);
            return false;
        }

        auto ip_string = address.ip_to_string();

        sockaddr_in windows_address;
        windows_address.sin_family = AF_INET;
        windows_address.sin_addr.s_addr = inet_addr(ip_string.c_str());
        windows_address.sin_port = htons(address.port);

        // INETADDR_SETLOOPBACK((SOCKADDR *)&windows_address);
        // SS_PORT((SOCKADDR *)&windows_address) = htons(address.port);

        auto res = ::connect(fd, (SOCKADDR *)&windows_address, sizeof(windows_address));
        if (res == SOCKET_ERROR) {
            if (WSAGetLastError() == WSAECONNREFUSED) {
                printf("Connection refused\n");
            }
            auto d = WSAGetLastError();
            int j = 0;
        }

        if (SOCKET_ERROR == res) {
            if (WSAEWOULDBLOCK != WSAGetLastError()) {
                CLOSESOCK(fd);
                return false;
            }
        }

        write(std::make_unique<Buffer>(),
              [&, this, on_connection = std::forward<Func>(on_connection)](auto buffer, auto error) mutable {
                  this->connected = true;
                  on_connection(*this, error);
              });
        return true;
    }

    // void register_socket_to_context(Context &context);

    bool is_connected() { return connected; }

    ~WindowsSocket() { CLOSESOCK(fd); }
};

using Socket = WindowsSocket;

class ListeningSocket {
    SOCKET mFd;
    std::function<void(Socket &&)> connection_accepted;

  public:
    template <typename Func>
    ListeningSocket(Func &&connection_accepted, SOCKET fd = INVALID_SOCKET)
        : mFd(fd)
        , connection_accepted(std::forward<Func>(connection_accepted)) {}

    ListeningSocket(SOCKET fd = INVALID_SOCKET)
        : mFd(fd)
        , connection_accepted([](Socket &&) {}) {}

    ListeningSocket(ListeningSocket &&ls)
        : connection_accepted(std::move(ls.connection_accepted))
        , mFd(ls.mFd) {
        ls.mFd = INVALID_SOCKET;
    }

    template <typename Func> void set_connection_accepted(Func &&func) {
        connection_accepted = std::forward<Func>(func);
    }

    auto FD() { return mFd; }

    bool start_listening(int port) {
        // already listening
        if (mFd != INVALID_SOCKET) {
            return false;
        }

        SOCKADDR_STORAGE addr = {0};
        addr.ss_family = AF_INET;
        INETADDR_SETANY((SOCKADDR *)&addr);
        SS_PORT((SOCKADDR *)&addr) = htons(port);

        if (INVALID_SOCKET == (mFd = socket(AF_INET, SOCK_STREAM, 0))) {
            return false;
        }
        ULONG uNonBlockingMode = 1;

        if (SOCKET_ERROR == ioctlsocket(mFd, FIONBIO, &uNonBlockingMode)) {
            return false;
        }

        auto err = bind(mFd, (SOCKADDR *)&addr, sizeof(addr));
        if (SOCKET_ERROR == err) {
            printf("Failed to bind %d\n", err);
            return false;
        }

        if (SOCKET_ERROR == listen(mFd, 1)) {
            return false;
        }

        return true;
    }

    void accept_connection() {
        auto fd_new_connection = accept(mFd, NULL, NULL);
        if (fd_new_connection != INVALID_SOCKET) {
            connection_accepted(Socket(fd_new_connection));
        }
    }

    ~ListeningSocket() { CLOSESOCK(mFd); }
};
