#pragma once
#include "coroutine.h"
#include "async_task.h"

class AsyncOperation {
    coroutine_handle<> handle;

  public:
    // after this operation, no memory must be accessed,
    // as it could have been destroyed in the coroutine finishing
    void available() {
        if (handle) {
            auto temp_handle = handle;
            handle = {};
            temp_handle.resume();
            // pass this point our memory could be destroyed
            // if the handle has been completed

            if (!temp_handle.done()) {
                // safe to access our memory since coroutine is not done
                handle = temp_handle;
            }
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
    SOCKET fd;
    std::vector<char> overflow;
    size_t overflow_start = 0;
    ReadAsyncOperation(SOCKET fd)
        : fd(fd) {}

    int poll_op(char *data, int n) {
        auto overfill_size = overflow.size() - overflow_start;

        int amount = 0;
        if (overfill_size != 0) {
            auto num_to_write = std::min((size_t)n, overfill_size);
            memcpy(data, overflow.data() + overflow_start, num_to_write);
            n -= num_to_write;
            data += num_to_write;
            overflow_start += num_to_write;
            amount += num_to_write;
        }

        if (n == 0) {
            return amount;
        }

        auto ret = recv(fd, data, n, 0);
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
    SOCKET fd;
    WriteAsyncOperation(SOCKET fd)
        : fd(fd) {}

    int poll_op(const char *data, int n) {
        int w;
        return send(fd, data, n, 0);
    }
    AsyncTask<> operator()(const char *data, int n) { return async_stream((char *)data, n, *this); }
};