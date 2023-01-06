#pragma once
#include <functional>
#include <queue>
#include <memory>
#include <optional>
#include "buffer.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <mstcpip.h>

#include <stdio.h>

class Context;

struct Error
{
};

using Buffer_Ptr = std::unique_ptr<Buffer>;

template <typename Base_Op>
class Operation : public Base_Op
{
    struct Request
    {
        std::function<void(Buffer_Ptr, bool)> callback;
        Buffer_Ptr buffer;
        template<typename Func>
        Request(Func&& callback, Buffer_Ptr buffer)
            : callback(std::forward<Func>(callback)), buffer(std::move(buffer)) {}

        Request(const Request &) = default;
        Request(Request &&) = default;
        Request &operator=(const Request &) = default;
    };
    std::queue<Request> operation_requests;
    bool can_perform;

public:
    template <typename... Args>
    Operation(bool can_perform, Args &&...args) : can_perform(can_perform),
                                                  Base_Op(std::forward<Args>(args)...) {}

    template <typename Func1>
    void request(Buffer_Ptr b, Func1 &&callback)
    {
        // if (can_perform)
        // {
        //     if (operation(*b))
        //     {
        //         callback(std::move(b), false);
        //         return;
        //     }
        // }

        operation_requests.emplace(std::forward<Func1>(callback), std::move(b));
    }

    void check_requests()
    {
        while (!operation_requests.empty())
        {
            auto &front = operation_requests.front();
            if (operation(*front.buffer))
            {
                front.callback(std::move(front.buffer), false);
                operation_requests.pop();
            }
            else
            {
                break;
            }
        }
    }

private:
    bool operation(Buffer &b)
    {
        for (;;)
        {
            auto [data, size] = b.contiguous();
            if (size == 0)
                return true;
            int num_written = base_operation(data, size);
            b.advance(num_written);
            if (num_written != size)
                return false;
        }
    }
};

struct _WriteOperation
{
    SOCKET fd;
    _WriteOperation(SOCKET fd)
        : fd(fd) {}

    int base_operation(char *d, int n)
    {
        return send(fd, d, n, NULL);
    }
};
using WriteOperation = Operation<_WriteOperation>;

struct _ReadOperation
{
    SOCKET fd;
    _ReadOperation(SOCKET fd)
        : fd(fd) {}

    int base_operation(char *d, int n)
    {
        return recv(fd, d, n, NULL);
    }
};

using ReadOperation = Operation<_ReadOperation>;

class Socket
{
    WriteOperation writeOps;
    ReadOperation readOps;
    bool listening;
    SOCKET fd;

public:
    Socket(SOCKET fd = INVALID_SOCKET) : writeOps(false, fd), readOps(false, fd), fd(fd)
    {
    }
    Socket(const Socket& s) = delete;
    Socket(Socket&& s) = default;

    template <typename Func>
    void read(Buffer_Ptr &&buffer, Func &&func)
    {
        readOps.request(std::move(buffer), std::forward<Func>(func));
    }

    void reads_available()
    {
        readOps.check_requests();
    }

    template <typename Func>
    void write(Buffer_Ptr &&buffer, Func &&func)
    {
        writeOps.request(std::move(buffer), std::forward<Func>(func));
    }

    void write_available()
    {
        writeOps.check_requests();
    }

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

        readOps.fd = fd;
        writeOps.fd = fd;


      

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
                     { 
                        on_connection(*this, error);
                     });
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

        if (INVALID_SOCKET == (lsock = socket(AF_INET6,
                                              SOCK_STREAM,
                                              0)))
        {
            return {};
        }
        ULONG uNonBlockingMode = 1;

        if (SOCKET_ERROR == ioctlsocket(lsock,
                                        FIONBIO,
                                        &uNonBlockingMode))
        {
            return {};
        }

        if (SOCKET_ERROR == bind(lsock,
                                 (SOCKADDR *)&addr,
                                 sizeof(addr)))
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
        if(fd_new_connection != INVALID_SOCKET){
            connection_accepted(Socket(fd_new_connection));
        }else{
            int w;
        }
    }

    void register_socket_to_context(Context &context);
};
