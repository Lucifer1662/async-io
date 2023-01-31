#pragma once
#include "coroutine.h"
#include "async_task.h"
#include "os_socket.h"
#include <cstring>

namespace AsyncOperation {

AsyncTask<int> read(OS::SOCKET fd, SocketContext &context, char *data, int n) {
    struct Awaitable : public SocketContextHandler {
        coroutine_handle<> handle;
        bool failed = false;

        bool await_ready() const noexcept { return false; }
        void await_resume() const noexcept {}
        void await_suspend(coroutine_handle<> h) noexcept { handle = h; }

        void on_event(Flag f, SocketContext &context) override {
            if (f.isRead())
                handle.resume();
        };

        void on_removed() override {
            if (handle && !handle.done()) {
                failed = true;
                handle.resume();
            }
        }
    };

    auto awaitable = Awaitable();

    auto amount = OS::read(fd, data, n);

    if (amount > 0) {
        co_return amount;
    }

    context.add_socket(fd, &awaitable);
    co_await awaitable;

    co_return = OS::read(fd, data, n);

    if (awaitable.failed) {
        throw std::exception();
    }
}

AsyncTask<int> write(OS::SOCKET fd, SocketContext &context, char *data, int n) {
    struct Awaitable : public SocketContextHandler {
        coroutine_handle<> handle;
        bool failed = false;

        bool await_ready() const noexcept { return false; }
        void await_resume() const noexcept {}
        void await_suspend(coroutine_handle<> h) noexcept { handle = h; }

        void on_event(Flag f, SocketContext &context) override {
            if (f.isRead())
                handle.resume();
        };

        void on_removed() override {
            if (handle && !handle.done()) {
                failed = true;
                handle.resume();
            }
        }
    };

    auto awaitable = Awaitable();

    auto amount = OS::read(fd, data, n);

    if (amount > 0) {
        co_return amount;
    }

    context.add_socket(fd, &awaitable);
    co_await awaitable;

    co_return = OS::read(fd, data, n);

    if (awaitable.failed) {
        throw std::exception();
    }
}

}   // namespace AsyncOperation

class AsyncOperation {
    coroutine_handle<> handle;

  public:
    AsyncOperation() = default;
    AsyncOperation(const AsyncOperation &) = delete;
    AsyncOperation(AsyncOperation &&old)
        : handle(old.handle) {
        old.handle = {};
    };

    AsyncOperation &operator=(AsyncOperation &&old) {
        handle = old.handle;
        old.handle = {};
        return *this;
    }

    void destroy_corourtine() {
        if (handle) {
            handle.destroy();
        }
    }

    ~AsyncOperation() { destroy_corourtine(); }

    // after this operation, no memory must be accessed,
    // as it could have been destroyed in the coroutine finishing
    void available() {
        if (handle) {
            auto temp_handle = handle;
            handle = {};
            temp_handle.resume();
            // pass this point our memory could be destroyed
            // if the handle has been completed

            // if (!temp_handle.done()) {
            //     // safe to access our memory since coroutine is not done
            //     handle = temp_handle;
            // }
        }
    }

    auto wait() {
        struct Awaitable {
            AsyncOperation &operation;
            constexpr bool await_ready() const noexcept { return false; }
            void await_resume() const noexcept {}
            void await_suspend(coroutine_handle<> h) { operation.handle = h; }
        };

        return Awaitable{*this};
    }
};

template <typename AsyncOperation> AsyncTask<> async_stream(char *data, int n, AsyncOperation &operation) {
    while (n != 0) {
        auto num = operation.poll_op(data, n);
        if (num == -1) {
            num = 0;
        }
        n -= num;
        data += num;

        if (num < n) {
            co_await operation.wait();
        }
    }
}

struct ReadAsyncOperation : public AsyncOperation {
    OS::SOCKET fd;
    std::vector<char> overflow;
    size_t overflow_start = 0;
    ReadAsyncOperation(OS::SOCKET fd)
        : fd(fd) {}

    int poll_op(char *data, int n) {
        auto overfill_size = overflow.size() - overflow_start;

        int amount = 0;
        if (overfill_size != 0) {
            auto num_to_write = std::min((size_t)n, overfill_size);
            std::memcpy(data, overflow.data() + overflow_start, num_to_write);
            n -= num_to_write;
            data += num_to_write;
            overflow_start += num_to_write;
            amount += num_to_write;
        }

        if (n == 0) {
            return amount;
        }

        auto ret = OS::read(fd, data, n);
        if (ret == -1)
            ret = 0;

        return amount + ret;
    }

    AsyncTask<> operator()(char *data, int n) { return async_stream(data, n, *this); }

    void assign_overflow(char *data, int n) {
        overflow.assign(data, data + n);
        overflow_start = 0;
    }
};

struct WriteAsyncOperation : public AsyncOperation {
    OS::SOCKET fd;
    WriteAsyncOperation(OS::SOCKET fd)
        : fd(fd) {}

    int poll_op(const char *data, int n) { return OS::send(fd, data, n); }
    AsyncTask<> operator()(const char *data, int n) { return async_stream((char *)data, n, *this); }
};

#include <iostream>
#include <errno.h>

struct AcceptAsyncOperation : public AsyncOperation {
    OS::SOCKET fd;
    AcceptAsyncOperation(OS::SOCKET fd)
        : fd(fd) {}

    OS::SOCKET poll_op(int port) { return OS::accept(fd, port); }
    AsyncTask<OS::SOCKET> operator()(int port) {
        for (;;) {
            auto socket = poll_op(port);
            if (socket != OS::INVALID_SOCKET) {
                co_return socket;
            }
            co_await wait();
        }
    }
};