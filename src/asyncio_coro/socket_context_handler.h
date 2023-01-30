#pragma once

class SocketContext;

struct SocketContextHandler {
    virtual void on_added() {}
    virtual void on_read(SocketContext &context, int i) {}
    virtual void on_write(SocketContext &context, int i) {}
    virtual void on_error(SocketContext &context, int i) {}
    virtual void on_removed() {}
    virtual ~SocketContextHandler() {}
};