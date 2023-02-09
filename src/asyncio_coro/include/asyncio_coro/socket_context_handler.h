#pragma once

class SocketContext;

#if WIN32
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

#else
#include <sys/epoll.h>

struct Flag {
    uint32_t flag;
    Flag(uint32_t flag = 0)
        : flag(flag) {}

    bool isRead() const { return flag & EPOLLIN; }
    bool isWrite() const { return flag & EPOLLOUT; }
    bool isError() const { return flag & EPOLLERR; }
    void setRead() { flag |= EPOLLIN; }
    void setWrite() { flag |= EPOLLOUT; }
    void setReadIf(bool condition) {
        if (condition)
            setRead();
    }
    void setWriteIf(bool condition) {
        if (condition)
            setWrite();
    }
};

#endif

struct SocketContextHandler {
    virtual void on_event(Flag f, SocketContext &context) {}
    virtual void on_removed() {}
    virtual ~SocketContextHandler() {}
};