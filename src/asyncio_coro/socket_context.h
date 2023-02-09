#pragma once
#include "os_socket.h"
#include <vector>
#include <unordered_map>
#include "socket_context_handler.h"
#include "async_task.h"
#include <memory>

#if WIN32
struct Poll {
    SOCKET fd;
    Flag events;
    Flag revents;
};
#endif

struct ReadAwaitable;
struct WriteAwaitable;

class SocketContext {
    struct Awaitable;
    struct Handlers {
        Awaitable *read = nullptr;
        Awaitable *write = nullptr;
        OS::SOCKET fd;
    };
    std::unordered_map<OS::SOCKET, std::unique_ptr<Handlers>> sockets;

#if WIN32
    std::vector<Poll> socket_events;
    std::vector<SocketContextHandler *> sockets;

#else

    std::vector<epoll_event> events;

    OS::SOCKET epollfd;
#endif

    void remove_sockets();

    bool subscribe_for_read(OS::SOCKET fd, Awaitable *callback);
    bool subscribe_for_write(OS::SOCKET fd, Awaitable *callback);
    bool update_subscription(OS::SOCKET fd, Handlers &handlers, bool is_new);
    Handlers &get_handlers(OS::SOCKET fd, bool &is_new);
    void remove_handlers(OS::SOCKET fd);

  public:
    SocketContext(size_t max_events = 256);

    bool is_good();
    bool poll();
    void set_read_subscription(int socket_i, bool is_on);
    void set_write_subscription(int socket_i, bool is_on);

    AsyncTask<bool> read(OS::SOCKET fd, char *data, int n);
    AsyncTask<> wait_for_read(OS::SOCKET fd);
    AsyncTask<> wait_for_write(OS::SOCKET fd);

    ~SocketContext();
};
