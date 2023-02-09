#pragma once
#include <optional>
#include "socket.h"
#include "os_socket.h"
#include "async_operation.h"
#include "socket_context_handler.h"
#include "async_task.h"
#include <memory>

class SocketContext;

class ListeningSocket {
    SocketContext &context;
    OS::SOCKET socket;
    int port;
    AcceptAsyncOperation accept_operation;

  public:
    ListeningSocket(SocketContext &context, OS::SOCKET socket);
    static std::optional<ListeningSocket> create(SocketContext &context);

    bool start_listening(int port);
    AsyncTask<Socket> accept();
};
