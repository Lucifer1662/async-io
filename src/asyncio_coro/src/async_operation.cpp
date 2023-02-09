#include "asyncio_coro/async_operation.h"
#include "asyncio_coro/socket_context.h"

namespace AsyncOperation {

AsyncTask<int> read(OS::SOCKET fd, SocketContext &context, char *data, int n) {
    auto amount = OS::read(fd, data, n);

    if (amount > 0) {
        co_return amount;
    }

    co_await context.wait_for_read(fd);

    co_return OS::read(fd, data, n);
}
}   // namespace AsyncOperation

int OverflowOperation::operator()(char *&data, int &n) {
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
    return amount;
}

void OverflowOperation::assign(char *data, int n) {
    overflow.assign(data, data + n);
    overflow_start = 0;
}

ReadAsyncOperation::ReadAsyncOperation(OS::SOCKET fd, SocketContext &context)
    : fd(fd)
    , context(context) {}

int ReadAsyncOperation::poll_op(char *data, int n) {
    int amount = overflow(data, n);

    if (n == 0) {
        return amount;
    }

    auto ret = OS::read(fd, data, n);
    if (ret == -1)
        ret = 0;

    return amount + ret;
}

AsyncTask<> ReadAsyncOperation::wait() { return context.wait_for_read(fd); }

AsyncTask<> ReadAsyncOperation::operator()(char *data, int n) { return async_stream(data, n, *this); }

void ReadAsyncOperation::assign_overflow(char *data, int n) { overflow.assign(data, n); }

AsyncTask<> WriteAsyncOperation::wait() { return context.wait_for_write(fd); }

WriteAsyncOperation::WriteAsyncOperation(OS::SOCKET fd, SocketContext &context)
    : fd(fd)
    , context(context) {}

AcceptAsyncOperation::AcceptAsyncOperation(OS::SOCKET fd, SocketContext &context)
    : fd(fd)
    , context(context) {}

AsyncTask<> AcceptAsyncOperation::wait() { return context.wait_for_read(fd); }

AsyncTask<OS::SOCKET> AcceptAsyncOperation::operator()(int port) {
    for (;;) {
        auto socket = poll_op(port);
        if (socket != OS::INVALID_SOCKET) {
            co_return socket;
        }
        co_await wait();
    }
}