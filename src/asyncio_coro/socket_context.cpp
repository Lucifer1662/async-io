#include "socket_context.h"
#include <iostream>
#include "os_socket.h"

#if WIN32

bool SocketContext::add_socket(OS::SOCKET fd, SocketContextHandler *callback) {
    Poll d{fd, POLLRDNORM | POLLWRNORM, 0};
    socket_events.push_back(d);
    sockets.push_back(callback);
    callback->on_added();
    return true;
}

bool SocketContext::remove_socket(OS::SOCKET fd) {
    auto it = std::find_if(socket_events.begin(), socket_events.end(), [=](auto &e) { return e.fd == fd; });
    std::cout << "Remove " << fd << std::endl;
    // if (it != socket_events.end()) {
    //     it->fd = INVALID_SOCKET;
    // }
    return true;
}

void SocketContext::remove_sockets() {

    for (size_t i = 0; i < socket_events.size();) {
        if (socket_events[i].fd == INVALID_SOCKET) {

            // delete socket event
            socket_events[i] = socket_events.back();
            socket_events.pop_back();

            sockets[i]->on_removed();
            // delete socket callback
            sockets[i] = sockets.back();
            sockets.pop_back();
        } else {
            i++;
        }
    }
}

bool SocketContext::poll() {
    if (socket_events.empty())
        return false;
    if (SOCKET_ERROR != WSAPoll((WSAPOLLFD *)&socket_events.front(), socket_events.size(), 0)) {

        auto size = socket_events.size();
        for (size_t i = 0; i < size; i++) {
            if (socket_events[i].fd != OS::INVALID_SOCKET) {
                if (socket_events[i].revents.isRead()) {
                    sockets[i]->on_read(*this, i);
                }
            }
            if (socket_events[i].fd != OS::INVALID_SOCKET) {
                if (socket_events[i].revents.isWrite()) {
                    std::cout << "Accessed " << socket_events[i].fd << std::endl;
                    sockets[i]->on_write(*this, i);
                }
            }
        }

        remove_sockets();
    }
}

void SocketContext::check_sockets() {}

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

SocketContext::SocketContext(int max_events) { global.start(); }

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

#else

SocketContext::SocketContext(size_t max_events) {
    epollfd = epoll_create1(0);

    // struct epoll_event events1;

    // std::cout << "Error: " << errno << std::endl;

    // // nfds = epoll_wait(epollfd, events.data(), events.size(), 1);
    // int nfds = epoll_wait(epollfd, &events1, 1, 1);

    // std::cout << "Polled: " << nfds << std::endl;

    // if (nfds == -1) {
    //     std::cout << "Error: " << errno << std::endl;
    // }

    events.assign(max_events, epoll_event());
}

bool SocketContext::is_good() { return epollfd == OS::INVALID_SOCKET; }

bool SocketContext::add_socket(OS::SOCKET fd, SocketContextHandler *callback) {
    epoll_event ev;
    ev.events = EPOLLIN | EPOLLOUT;
    ev.data.ptr = callback;
    sockets[fd] = callback;
    return epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1;
}

bool SocketContext::remove_socket(OS::SOCKET fd) {
    epoll_event ev;
    ev.data.fd = fd;
    sockets.erase(fd);
    return epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev) == -1;
}

#include "errno.h"

bool SocketContext::poll() {
    struct epoll_event events1;

    auto nfds = epoll_wait(epollfd, events.data(), events.size(), 1);

    std::cout << "Polled: " << nfds << std::endl;

    if (nfds == -1) {
        std::cout << "Error: " << errno << std::endl;
        return false;
    }

    for (size_t i = 0; i < nfds; i++) {
        auto event = events[i];
        auto handler = (SocketContextHandler *)events[i].data.ptr;
        handler->on_event(events[i].events, *this);
    }

    return true;
}

SocketContext::~SocketContext() {
    for (auto [fd, handler] : sockets) {
        handler->on_removed();
    }
}

AsyncTask<bool> SocketContext::read(OS::SOCKET fd, char *data, int n) {
    struct Awaitable : public SocketContextHandler {
        coroutine_handle<> handle;
        bool failed = false;

        bool await_ready() const noexcept { return false; }
        void await_resume() const noexcept {}
        void await_suspend(coroutine_handle<> h) noexcept { handle = h; }

        void on_event(Flag f, SocketContext &context) override { handle.resume(); };

        void on_removed() override {
            if (handle && !handle.done()) {
                failed = true;
                handle.resume();
            }
        }
    };

    auto awaitable = Awaitable();

    add_socket(fd, &awaitable);
    co_await awaitable;
    if (awaitable.failed) {
        throw std::exception();
    }
}

#endif