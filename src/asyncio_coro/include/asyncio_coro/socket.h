#pragma once
#include "os_socket.h"
#include "socket_context_handler.h"
#include "async_operation.h"
#include <optional>
#include <memory>
#include "async_task.h"

class SocketContext;
class IPAddress;

class Socket {
    SocketContext &context;
    OS::SOCKET fd;

  public:
    ReadAsyncOperation read_operation;
    WriteAsyncOperation write_operation;

    Socket(SocketContext &context, OS::SOCKET fd);
    Socket(const Socket &) = delete;
    Socket(Socket &&) = default;

    Socket &operator=(Socket &&) = default;

    static std::optional<Socket> create(SocketContext &context);

    AsyncTask<> connect(const IPAddress &address);

    OS::SOCKET FD() { return fd; }
};
