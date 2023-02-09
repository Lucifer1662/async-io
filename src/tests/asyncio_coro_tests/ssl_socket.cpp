#include <gtest/gtest.h>

#include <asyncio_coro/async_task.h>
#include <asyncio_coro/timer_context.h>
#include <asyncio_coro/socket_context.h>
#include <asyncio_coro/interval.h>
#include <asyncio_coro/linked_socket_context.h>
#include <iostream>

TEST(SSL_Socket_Test, ClientServer) {
    TimerContext context;
    SocketContext socketContext;

    link_socket_to_timer_context(context, socketContext, std::chrono::milliseconds(100));

    context.step();
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
