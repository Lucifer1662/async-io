#pragma once
#include <functional>
#include <queue>
#include <memory>
#include <optional>
#include "buffer/buffer.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>

#include <stdio.h>
#include "async_operation.h"

class Context;

struct Error
{
};


struct WriteOperation
{
    SOCKET fd;
    WriteOperation(SOCKET fd)
        : fd(fd) {}

    int operator()(char *d, int n)
    {
        return send(fd, d, n, NULL);
    }
};

using AsyncWriteOperation = AsyncOperation<WriteOperation>;

struct ReadOperation
{
    SOCKET fd;
    ReadOperation(SOCKET fd)
        : fd(fd) {}

    int operator()(char *d, int n)
    {
        return recv(fd, d, n, NULL);
    }
};

using AsyncReadOperation = AsyncOperation<ReadOperation>;

class Socket
{
    AsyncWriteOperation writeOps;
    AsyncReadOperation readOps;
    bool listening;
    SOCKET fd;

public:
    Socket(SOCKET fd = INVALID_SOCKET) : writeOps(false, fd), readOps(false, fd), fd(fd)
    {
    }
    Socket(const Socket &s) = delete;
    Socket(Socket &&s) = default;

    template <typename Func>
    void read(Buffer_Ptr &&buffer, Func &&func) { readOps.request(std::move(buffer), std::forward<Func>(func)); }
    template <typename Func>
    void write(Buffer_Ptr &&buffer, Func &&func) { writeOps.request(std::move(buffer), std::forward<Func>(func)); }

    bool reads_available() { return readOps.check_requests(); }
    bool write_available() { return writeOps.check_requests(); }

    bool wants_to_read() { return readOps.wants_more(); }
    bool wants_to_write() { return writeOps.wants_more(); }

    void register_socket_to_context(Context &context);

    template <typename Func>
    bool connect(int port, Func &&on_connection)
    {
        auto CLOSESOCK = [](auto s)
        {
            if (INVALID_SOCKET != s)
            {
                closesocket(s);
                s = INVALID_SOCKET;
            }
        };

        fd = INVALID_SOCKET;
        SOCKADDR_STORAGE addrLoopback = {0};
        INT ret = 0;
        ULONG uNonBlockingMode = 1;

        if (INVALID_SOCKET == (fd = socket(AF_INET6,
                                           SOCK_STREAM,
                                           0)))
        {
            CLOSESOCK(fd);
            return false;
        }

        readOps.base_operation.fd = fd;
        writeOps.base_operation.fd = fd;

        addrLoopback.ss_family = AF_INET6;
        INETADDR_SETLOOPBACK((SOCKADDR *)&addrLoopback);
        SS_PORT((SOCKADDR *)&addrLoopback) = htons(port);

        if (SOCKET_ERROR == ::connect(fd,
                                      (SOCKADDR *)&addrLoopback,
                                      sizeof(addrLoopback)))
        {
            if (WSAEWOULDBLOCK != WSAGetLastError())
            {
                CLOSESOCK(fd);
                return false;
            }
        }

        if (SOCKET_ERROR == ioctlsocket(fd,
                                        FIONBIO,
                                        &uNonBlockingMode))
        {
            CLOSESOCK(fd);
            return false;
        }

        write(std::make_unique<Buffer>(), [=](auto buffer, auto error)
              { on_connection(*this, error); });
        return true;
    }
};

struct ListeningSocket
{
    SOCKET mFd;
    std::function<void(Socket &&)> connection_accepted;

    template <typename Func>
    ListeningSocket(SOCKET fd, Func &&connection_accepted)
        : mFd(fd), connection_accepted(std::forward<Func>(connection_accepted))
    {
    }

    template <typename Func>
    static std::optional<ListeningSocket> create_listening_socket(int port, Func &&connection_accepted)
    {
        SOCKET lsock = INVALID_SOCKET;

        SOCKADDR_STORAGE addr = {0};
        addr.ss_family = AF_INET6;
        INETADDR_SETANY((SOCKADDR *)&addr);
        SS_PORT((SOCKADDR *)&addr) = htons(port);

        if (INVALID_SOCKET == (lsock = socket(AF_INET6, SOCK_STREAM, 0)))
        {
            return {};
        }
        ULONG uNonBlockingMode = 1;

        if (SOCKET_ERROR == ioctlsocket(lsock, FIONBIO, &uNonBlockingMode))
        {
            return {};
        }

        if (SOCKET_ERROR == bind(lsock, (SOCKADDR *)&addr, sizeof(addr)))
        {
            return {};
        }

        if (SOCKET_ERROR == listen(lsock, 1))
        {
            return {};
        }

        return ListeningSocket(lsock, std::forward<Func>(connection_accepted));
    }

    void accept_connection()
    {
        auto fd_new_connection = accept(mFd, NULL, NULL);
        if (fd_new_connection != INVALID_SOCKET)
        {
            connection_accepted(Socket(fd_new_connection));
        }
    }

    void register_socket_to_context(Context &context);
};
