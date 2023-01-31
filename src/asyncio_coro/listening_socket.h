#pragma once
#include <optional>
#include "socket.h"
#include "raw_socket.h"
#include "socket_context_handler.h"
#include "async_task.h"
#include <memory>

class SocketContext;

class ListeningSocket : public RawListeningSocket, SocketContextHandler {
    SocketContext &context;

  public:
    ListeningSocket(SocketContext &context, RawListeningSocket socket);
    ListeningSocket(const ListeningSocket &) = delete;
    ListeningSocket(ListeningSocket &&) = delete;
    static std::optional<ListeningSocket> create(SocketContext &context);

    static std::optional<std::unique_ptr<ListeningSocket>> create_ptr(SocketContext &context);

    void on_event(Flag f, SocketContext &context) override { accept_available(); }
    void on_removed() override { destroy_dependent_coroutines(); }

    AsyncTask<std::unique_ptr<Socket>> accept();

    ~ListeningSocket();
};
