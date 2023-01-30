#pragma once
#include "raw_socket.h"
#include "socket_context_handler.h"
#include <optional>
#include <memory>
#include "listening_socket.h"

class SocketContext;

class Socket : public RawSocket, SocketContextHandler {
    SocketContext &context;

  public:
    Socket(SocketContext &context, RawSocket socket);

    Socket(const Socket &) = delete;
    Socket(Socket &&) = delete;
    static std::optional<Socket> create(SocketContext &context);

    static std::optional<std::unique_ptr<Socket>> create_ptr(SocketContext &context);

    void on_read(SocketContext &context, int i) override { read_available(); }

    void on_write(SocketContext &context, int i) override { write_available(); }

    AsyncTask<std::unique_ptr<Socket>> accept() {
        std::optional<Socket> new_socket;
        for (;;) {
            auto new_socket_opt = co_await RawSocket::accept();
            if (new_socket_opt.has_value()) {
                co_yield std::make_unique<Socket>(context, *new_socket_opt);
            } else {
                co_return {};
            }
        };
    }

    ~Socket();
};
