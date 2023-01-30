#pragma once
#include <WinSock2.h>
#include <vector>
#include "socket_context_handler.h"

struct Flag {
    short flag;
    Flag(short flag = 0)
        : flag(flag) {}

    bool isRead() const { return flag & POLLRDNORM; }
    bool isWrite() const { return flag & POLLWRNORM; }
    bool isError() const { return flag & POLLNVAL; }
    void setRead() { flag |= POLLRDNORM; }
    void setWrite() { flag |= POLLWRNORM; }
    void setReadIf(bool condition) {
        if (condition)
            setRead();
    }
    void setWriteIf(bool condition) {
        if (condition)
            setWrite();
    }
};

// for windows

// class SocketContext {
//     std::vector<WSAPOLLFD> socket_events;
//     std::vector<std::function<void(Flag, SocketContext &context, int i)>> sockets;

//   public:
//     template <typename Func> void add_socket(SOCKET fd, Func &&callback) {
//         WSAPOLLFD d = {0};
//         d.fd = fd;
//         d.events = POLLRDNORM | POLLWRNORM;
//         socket_events.push_back(d);
//         sockets.push_back(std::forward<Func>(callback));
//     }

//     void remove_socket(SOCKET fd) {
//         auto it = std::find_if(socket_events.begin(), socket_events.end(), [=](WSAPOLLFD e) { return e.fd == fd; });
//         if (it != socket_events.end()) {
//             // get corresponding index
//             auto i = std::distance(socket_events.begin(), it);

//             // delete socket event
//             *it = socket_events.back();
//             socket_events.pop_back();

//             // delete socket callback
//             sockets[i] = sockets.back();
//             sockets.pop_back();
//         }
//     }

//     bool poll() {
//         if (socket_events.empty())
//             return false;
//         return SOCKET_ERROR != WSAPoll(&socket_events.front(), socket_events.size(), 0);
//     }

//     void check_sockets() {
//         auto size = socket_events.size();
//         for (size_t i = 0; i < size; i++) {
//             sockets[i](socket_events[i].revents, *this, i);
//         }
//     }

//     void set_subscription(int socket_i, Flag flag) { socket_events[socket_i].events = flag.flag; }

//     void step() {
//         // if new events on socket process sockets
//         if (poll())
//             check_sockets();
//     }
// };

struct Poll {
    SOCKET fd;
    Flag events;
    Flag revents;
};

class SocketContext {
    std::vector<Poll> socket_events;
    std::vector<SocketContextHandler *> sockets;
    std::vector<SOCKET> sockets_to_remove;

    void remove_sockets();

  public:
    SocketContext();

    void add_socket(SOCKET fd, SocketContextHandler *callback);

    void remove_socket(SOCKET fd);

    bool poll();

    void check_sockets();

    void set_read_subscription(int socket_i, bool is_on);
    void set_write_subscription(int socket_i, bool is_on);

    void step();
};
