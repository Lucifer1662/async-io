#include "socket_context.h"

void SocketContext::step() {
    // if new events on socket process sockets
    if (poll())
        check_sockets();
}

void SocketContext::add_socket(SOCKET fd, SocketContextHandler *callback) {
    Poll d{fd, POLLRDNORM | POLLWRNORM, 0};
    socket_events.push_back(d);
    sockets.push_back(callback);
    callback->on_added();
}

void SocketContext::remove_socket(SOCKET fd) { sockets_to_remove.push_back(fd); }

void SocketContext::remove_sockets() {

    for (auto fd : sockets_to_remove) {

        auto it = std::find_if(socket_events.begin(), socket_events.end(), [=](Poll e) { return e.fd == fd; });
        if (it != socket_events.end()) {
            // get corresponding index
            auto i = std::distance(socket_events.begin(), it);

            // delete socket event
            *it = socket_events.back();
            socket_events.pop_back();

            sockets[i]->on_removed();
            // delete socket callback
            sockets[i] = sockets.back();
            sockets.pop_back();
        }
    }

    sockets_to_remove.clear();
}

bool SocketContext::poll() {
    if (socket_events.empty())
        return false;
    return SOCKET_ERROR != WSAPoll((WSAPOLLFD *)&socket_events.front(), socket_events.size(), 0);
}

void SocketContext::check_sockets() {
    auto size = socket_events.size();
    for (size_t i = 0; i < size; i++) {
        if (socket_events[i].revents.isRead()) {
            sockets[i]->on_read(*this, i);
        }
    }

    remove_sockets();

    size = socket_events.size();
    for (size_t i = 0; i < size; i++) {
        if (socket_events[i].revents.isWrite()) {
            sockets[i]->on_write(*this, i);
        }
    }

    remove_sockets();
}

struct AsyncioGlobal {
    WSADATA wsd;
    int nStartup = 0;
    int nErr = 0;

    bool start() {
        if (nStartup == 0) {
            nErr = WSAStartup(0x0202, &wsd);
            if (nErr) {
                WSASetLastError(nErr);
                return false;
            } else {
                nStartup++;
            }
        }

        return true;
    }

    ~AsyncioGlobal() {
        if (nStartup)
            WSACleanup();
    }
};

static AsyncioGlobal global;

SocketContext::SocketContext() { global.start(); }

void SocketContext::set_read_subscription(int socket_i, bool is_on) {
    Flag flag = socket_events[socket_i].events;
    flag.setReadIf(is_on);
    socket_events[socket_i].events = flag.flag;
}

void SocketContext::set_write_subscription(int socket_i, bool is_on) {
    Flag flag = socket_events[socket_i].events;
    flag.setWriteIf(is_on);
    socket_events[socket_i].events = flag.flag;
}
