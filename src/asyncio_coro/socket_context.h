#pragma once
#include "os_socket.h"
#include <vector>
#include <unordered_map>
#include "socket_context_handler.h"
#include "async_task.h"

#if WIN32
struct Poll {
    SOCKET fd;
    Flag events;
    Flag revents;
};
#endif

class SocketContext {
#if WIN32
    std::vector<Poll> socket_events;
    std::vector<SocketContextHandler *> sockets;

#else
    std::vector<epoll_event> events;
    std::unordered_map<OS::SOCKET, SocketContextHandler *> sockets;

    OS::SOCKET epollfd;
#endif

    void remove_sockets();

  public:
    SocketContext(size_t max_events = 256);

    bool is_good();
    bool add_socket(OS::SOCKET fd, SocketContextHandler *callback);
    bool remove_socket(OS::SOCKET fd);
    bool poll();
    void set_read_subscription(int socket_i, bool is_on);
    void set_write_subscription(int socket_i, bool is_on);

    AsyncTask<bool> read(OS::SOCKET fd, char *data, int n);

    ~SocketContext();
};
