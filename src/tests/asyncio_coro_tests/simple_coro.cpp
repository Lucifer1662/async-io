#include <gtest/gtest.h>

#include <asyncio_coro/async_task.h>
#include <asyncio_coro/timer_context.h>
#include <asyncio_coro/socket_context.h>
#include <asyncio_coro/interval.h>
#include <asyncio_coro/buffer.h>
#include <asyncio_coro/socket.h>
#include <asyncio_coro/listening_socket.h>
#include <asyncio_coro/linked_socket_context.h>

namespace SimpleCoroTest {

AsyncTask<> child_task(TimerContext &timerContext) {
    for (int i = 0; i < 3; i++) {
        std::cout << "Child Task" << std::endl;
        co_await async_wait_for_ms(10, timerContext);
    }
}

coroutine_handle<> global_handle;

AsyncTask<> never_ending_task() {
    struct Awaitable {
        constexpr bool await_ready() const noexcept { return false; }
        void await_resume() const noexcept {}
        void await_suspend(coroutine_handle<> h) { global_handle = h; }
    };
    auto awaitable = Awaitable();
    co_await awaitable;

    int w;
}

AsyncTask<> parent_task(TimerContext &timerContext) {
    co_await never_ending_task();

    for (int i = 0; i < 3; i++) {
        co_await async_wait_for_ms(30, timerContext);
        std::cout << "Parent Task" << std::endl;
        child_task(timerContext);
    }
    co_return;
}

AsyncTask<> socket_task(SocketContext &context) {
    auto socket = OS::create_tcp_socket();
    OS::non_blocking_socket(socket);
    if (socket) {
        auto address = IPAddress::loopback(1235);
        OS::connect(socket, address.ip_to_string().c_str(), address.port);

        char data[10];
        try {
            bool result = co_await context.read(socket, (char *)&data, 10);
        } catch (std::exception e) {
            int j = 0;
        }

        int w = 0;
    }
}

TEST(SimpleCoroTest, Start) {

    // TimerContext context;
    SocketContext socketContext;

    // auto task = parent_task(context);

    // never_ending_task();

    socket_task(socketContext);
    int w = 0;

    // context.step();
    // context.step();
    // context.step();
    // context.step();
    // context.step();
    // context.step();
}

}   // namespace SimpleCoroTest