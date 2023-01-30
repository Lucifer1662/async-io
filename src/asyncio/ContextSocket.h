#pragma once
#include <memory>
#include "socket/socket.h"
#include "context.h"

void register_socket_to_context(Context &context, Socket &socket);
void register_socket_to_context(Context &context, ListeningSocket &socket);

class ContextSocket {
    Context &context;
    std::shared_ptr<Socket> socket;

  public:
    ContextSocket(Context &context)
        : context(context)
        , socket(std::make_shared<Socket>()) {}
    ContextSocket(ContextSocket &&cs) = default;
    ContextSocket(const ContextSocket &cs) = default;
    ContextSocket &operator=(const ContextSocket &s) {
        context = s.context;
        socket = s.socket;
        return *this;
    };

    ContextSocket(Context &context, Socket &&_socket)
        : context(context)
        , socket(std::make_shared<Socket>(std::move(_socket))) {
        register_socket_to_context(context, *socket);
    }

    // on_connection is of signature void(Socket& socket, bool error)
    template <typename Func> bool connect(const IPAddress &address, Func &&on_connection) {
        bool connected = socket->connect(address, std::forward<Func>(on_connection));
        register_socket_to_context(context, *socket);
        return connected;
    }

    //----Pass through to socket----

    auto FD() const { return socket->FD(); }

    bool is_connected() { return socket->is_connected(); }

    template <typename Buffer_T, typename Func> void read(std::unique_ptr<Buffer_T> &&buffer, Func &&func) {
        socket->read(std::move(buffer), std::forward<Func>(func));
    }
    template <typename Buffer_T, typename Func> void write(std::unique_ptr<Buffer_T> &&buffer, Func &&func) {
        socket->write(std::move(buffer), std::forward<Func>(func));
    }

    bool reads_available() { return socket->reads_available(); }
    bool write_available() { return socket->write_available(); }

    size_t wants_to_read() { return socket->wants_to_read(); }
    size_t wants_to_write() { return socket->wants_to_write(); }

    //-----------------------------

    ~ContextSocket() {
        if (socket.use_count() == 1)
            context.remove_socket(FD());
    }
};

struct ContextListeningSocket {
    Context &context;
    std::shared_ptr<ListeningSocket> socket;
    template <typename Func>
    ContextListeningSocket(Context &context, Func &&connection_accepted)
        : context(context)
        , socket(std::make_shared<ListeningSocket>(
              [&, connection_accepted = std::forward<Func>(connection_accepted)](Socket &&socket) {
                  connection_accepted(ContextSocket(context, std::move(socket)));
              })) {}

    ContextListeningSocket(Context &context)
        : context(context)
        , socket(std::make_shared<ListeningSocket>()) {}

    ContextListeningSocket(const ContextListeningSocket &) = default;

    bool start_listening(int port) {
        bool server_is_good = socket->start_listening(port);
        if (server_is_good)
            register_socket_to_context(context, *socket);
        return server_is_good;
    }

    template <typename Func> bool start_listening(int port, Func &&connection_accepted) {
        set_connection_accepted(std::forward<Func>(connection_accepted));
        return start_listening(port);
    }

    template <typename Func> void set_connection_accepted(Func &&connection_accepted) {
        socket->set_connection_accepted(
            [&, connection_accepted = std::forward<Func>(connection_accepted)](Socket &&socket) {
                connection_accepted(ContextSocket(context, std::move(socket)));
            });
    }

    //----Pass through to socket----

    auto FD() { return socket->FD(); }

    void accept_connection() { socket->FD(); }

    //-----------------------------

    ~ContextListeningSocket() {
        if (socket.use_count() == 1)
            context.remove_socket(FD());
    }
};
