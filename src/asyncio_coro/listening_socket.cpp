#include "listening_socket.h"
#include "socket_context.h"

ListeningSocket::ListeningSocket(SocketContext &context, RawListeningSocket socket)
    : context(context)
    , RawListeningSocket(std::move(socket)) {
    printf("Create: %d\n", FD());

    context.add_socket(FD(), this);
}

std::optional<ListeningSocket> ListeningSocket::create(SocketContext &context) {
    auto socket = RawListeningSocket::create();
    if (!socket) {
        return {};
    }
    return std::make_optional<ListeningSocket>(context, std::move(*socket));
}

std::optional<std::unique_ptr<ListeningSocket>> ListeningSocket::create_ptr(SocketContext &context) {
    auto socket = RawListeningSocket::create();
    if (!socket) {
        return {};
    }
    return std::make_unique<ListeningSocket>(context, std::move(*socket));
}

AsyncTask<std::unique_ptr<Socket>> ListeningSocket::accept() {
    for (;;) {
        auto new_socket_opt = co_await RawListeningSocket::accept();
        if (new_socket_opt.has_value()) {
            co_yield std::make_unique<Socket>(context, std::move(*new_socket_opt));
        } else {
            co_return {};
        }
    }
}

ListeningSocket::~ListeningSocket() {
    printf("Destroyed: %d\n", FD());
    context.remove_socket(FD());
}
