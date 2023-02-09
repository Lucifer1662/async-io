#include "asyncio_coro/socket_context.h"
#include <iostream>
#include "asyncio_coro/os_socket.h"

struct SocketContext::Awaitable {
    coroutine_handle<> handle;
    bool failed = false;

    bool await_ready() const noexcept { return false; }
    void await_resume() const noexcept {}
    void await_suspend(coroutine_handle<> h) noexcept { handle = h; }

    void resume() { handle.resume(); }

    void on_removed() {
        if (handle && !handle.done()) {
            failed = true;
            handle.resume();
        }
    }
};

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
        auto handlers = (Handlers *)events[i].data.ptr;
        Flag flag(events[i].events);
        // if (handlers->read) {
        //     auto callback = handlers->read;
        //     handlers->read = nullptr;
        //     callback->resume();
        // }
        // if (handlers->write) {
        //     auto callback = handlers->write;
        //     handlers->write = nullptr;
        //     callback->resume();
        // }

        if (flag.isRead() && handlers->read) {
            auto callback = handlers->read;
            handlers->read = nullptr;
            callback->resume();
        }
        if (flag.isWrite() && handlers->write) {
            auto callback = handlers->write;
            handlers->write = nullptr;
            callback->resume();
        }

        if (!handlers->write && !handlers->read) {
            remove_handlers(handlers->fd);
        }
    }

    return true;
}

void SocketContext::remove_handlers(OS::SOCKET fd) {
    sockets.erase(fd);
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, nullptr) == -1;
}

SocketContext::~SocketContext() {
    for (auto &[fd, handler] : sockets) {
        if (handler->read)
            handler->read->on_removed();
        if (handler->write)
            handler->write->on_removed();
    }
}

bool SocketContext::subscribe_for_read(OS::SOCKET fd, SocketContext::Awaitable *callback) {
    bool is_new;
    auto &handler = get_handlers(fd, is_new);
    handler.read = callback;
    return update_subscription(fd, handler, is_new);
}
bool SocketContext::subscribe_for_write(OS::SOCKET fd, SocketContext::Awaitable *callback) {
    bool is_new;
    auto &handler = get_handlers(fd, is_new);
    handler.write = callback;
    return update_subscription(fd, handler, is_new);
}

SocketContext::Handlers &SocketContext::get_handlers(OS::SOCKET fd, bool &is_new) {
    auto it = sockets.find(fd);
    if (it != sockets.end()) {
        is_new = false;
        return *it->second;
    } else {
        is_new = true;
        auto [new_it, success] = sockets.insert({fd, std::make_unique<SocketContext::Handlers>()});
        new_it->second->fd = fd;
        return *new_it->second;
    }
}

bool SocketContext::update_subscription(OS::SOCKET fd, SocketContext::Handlers &handlers, bool is_new) {
    epoll_event ev;
    ev.events = 0;
    ev.data.ptr = &handlers;
    int op = is_new ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;

    if (handlers.read) {
        ev.events |= EPOLLIN;
    }

    if (handlers.write) {
        ev.events |= EPOLLOUT;
    }

    return epoll_ctl(epollfd, op, fd, &ev) == -1;
}

#endif

AsyncTask<> SocketContext::wait_for_read(OS::SOCKET fd) {

    auto awaitable = SocketContext::Awaitable();

    subscribe_for_read(fd, &awaitable);
    co_await awaitable;

    if (awaitable.failed) {
        throw std::exception();
    }
}

AsyncTask<> SocketContext::wait_for_write(OS::SOCKET fd) {
    auto awaitable = SocketContext::Awaitable();

    subscribe_for_write(fd, &awaitable);
    co_await awaitable;

    if (awaitable.failed) {
        throw std::exception();
    }
}
