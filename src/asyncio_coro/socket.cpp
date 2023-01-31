#include "socket.h"
#include "socket_context.h"

Socket::Socket(SocketContext &context, RawSocket socket)
    : context(context)
    , RawSocket(std::move(socket)) {
    printf("Create: %d\n", FD());

    context.add_socket(FD(), this);
}

std::optional<Socket> Socket::create(SocketContext &context) {
    auto socket = RawSocket::create();
    if (!socket) {
        return {};
    }
    return std::make_optional<Socket>(context, std::move(*socket));
}

std::optional<std::unique_ptr<Socket>> Socket::create_ptr(SocketContext &context) {
    auto socket = RawSocket::create();
    if (!socket) {
        return {};
    }
    return std::make_unique<Socket>(context, std::move(*socket));
}

AsyncTask<int> Socket::read(char *data, int n) {
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

    auto amount = OS::read(data, n);

    if (amount > 0) {
        co_return amount;
    }

    context.add_socket(fd, &awaitable);
    co_await awaitable;
    if (awaitable.failed) {
        throw std::exception();
    }
}

Socket::~Socket() {
    printf("Destroyed: %d\n", FD());
    context.remove_socket(FD());
}
