#include "ContextSocket.h"

void register_socket_to_context(Context &context, Socket &socket) {
    context.add_socket(socket.FD(), [&](Flag flag, Context &context, size_t socket_i) {
        if (flag.isRead()) {
            socket.reads_available();
        }
        if (flag.isWrite()) {
            socket.write_available();
        }

        Flag f;
        f.setReadIf(socket.wants_to_read());
        f.setWriteIf(socket.wants_to_write());
        context.set_subscription(socket_i, f);

        if (flag.isError()) {
            printf("Error\n");
        }
    });
}

void register_socket_to_context(Context &context, ListeningSocket &socket) {
    context.add_socket(socket.FD(), [&](Flag flag, Context &context, size_t socket_i) {
        if (flag.isRead()) {
            socket.accept_connection();
        }
    });
}
