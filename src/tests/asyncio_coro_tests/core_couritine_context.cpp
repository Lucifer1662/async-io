#include <gtest/gtest.h>

#include <asyncio_coro/async_task.h>
#include <asyncio_coro/timer_context.h>
#include <asyncio_coro/socket_context.h>
#include <asyncio_coro/interval.h>
#include <asyncio_coro/buffer.h>
#include <asyncio_coro/socket.h>
#include <asyncio_coro/listening_socket.h>
#include <asyncio_coro/linked_socket_context.h>
#include <asyncio_coro/IpAddress.h>

AsyncTask<> child_task(TimerContext &timerContext) {
    for (int i = 0; i < 2; i++) {
        std::cout << "Child Task" << std::endl;
        co_await async_wait_for_ms(10, timerContext);
    }
}

AsyncTask<> parent_task(TimerContext &timerContext) {
    for (int i = 0; i < 3; i++) {
        co_await async_wait_for_ms(30, timerContext);
        std::cout << "Parent Task" << std::endl;
        co_await child_task(timerContext);
    }
}

// auto link_socket_to_socket_context(SocketContext &context, Socket &socket) {
//     context.add_socket(socket.FD(), [&](Flag flag, SocketContext &, auto socket_id) {
//         if (flag.isRead()) {
//             socket.read_available();
//         }
//         if (flag.isWrite()) {
//             socket.write_available();
//         }
//     });

//     return [fd = socket.FD(), &context]() { context.remove_socket(fd); }
// }

// auto link_socket_to_socket_context(SocketContext &context, RawSocket &socket) {

//     context.add_socket(socket.FD(), [&](Flag flag, SocketContext &, auto socket_id) {
//         if (flag.isRead()) {
//             socket.read_available();
//         }
//         if (flag.isWrite()) {
//             socket.write_available();
//         }
//     });

//     return [fd = socket.FD(), &context]() { context.remove_socket(fd); }
// }

AsyncTask<> handle_new_connection(Socket socket) {
    co_await write_async_c_string("hello there", socket.write_operation);
    std::cout << "Said Hello" << std::endl;
}

AsyncTask<> accept_connection(ListeningSocket &listening_socket) {
    for (;;) {
        Socket new_connection = co_await listening_socket.accept();
        std::cout << "New Connection" << std::endl;
        handle_new_connection(std::move(new_connection));
    };
}

static AsyncTask<> socket_read_task(SocketContext &socketContext) {
    auto socket_opt = Socket::create(socketContext);
    auto socket_opt1 = ListeningSocket::create(socketContext);

    if (socket_opt) {
        auto &socket = *socket_opt;
        auto &socket1 = *socket_opt1;

        auto listen_success = socket1.start_listening(1235);
        if (listen_success) {
            accept_connection(socket1);

            co_await socket.connect(IPAddress::loopback(1235));

            std::cout << "Connected" << std::endl;

            std::string str = co_await read_async_c_string(socket.read_operation);

            std::cout << "Message: " << str << std::endl;
        }
    }
}

TEST(Coroutine_ClientServer, Start) {

    TimerContext context;
    SocketContext socketContext;

    link_socket_to_timer_context(context, socketContext, std::chrono::milliseconds(100));
    // context.add_timer(wait_for_milliseconds(200), []() { std::cout << "Hello World" << std::endl; });

    // parent_task(context);

    AsyncTask<> task = socket_read_task(socketContext);

    // wait_for_ever();
    // char *d;
    // d = (char *)malloc(100);

    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
}
