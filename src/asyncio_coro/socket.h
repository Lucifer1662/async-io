#pragma once
#include "raw_socket.h"
#include "socket_context_handler.h"
#include <optional>
#include <memory>

class SocketContext;

class Socket : public RawSocket, SocketContextHandler {
    SocketContext &context;

  public:
    Socket(SocketContext &context, RawSocket socket);

    Socket(const Socket &) = delete;
    Socket(Socket &&) = delete;
    static std::optional<Socket> create(SocketContext &context);

    static std::optional<std::unique_ptr<Socket>> create_ptr(SocketContext &context);

    void on_event(Flag f, SocketContext &context) override {
        if (f.isRead()) {
            read_available();
        }
        if (f.isWrite()) {
            write_available();
        }
    }

    ~Socket();

    void on_removed() override { destroy_dependent_coroutines(); }
};
