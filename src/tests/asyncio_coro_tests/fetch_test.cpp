#include <gtest/gtest.h>

#include <asyncio_coro/async_task.h>
#include <asyncio_coro/timer_context.h>
#include <asyncio_coro/socket_context.h>
#include <asyncio_coro/interval.h>
#include <asyncio_coro/linked_socket_context.h>
#include <asyncio_coro/http/fetch.h>
#include <iostream>

static AsyncTask<> fetch_test(SocketContext &socketContext) {
    Http::FetchParams params;
    // params.url.host = "www.google.com";
    params.url.host = "www.testingmcafeesites.com";
    // params.url.host = "127.0.0.1";
    params.url.path = "/";

    try {
        auto response = co_await Http::fetch(params, socketContext);
        std::cout << response.body << std::endl;

    } catch (const std::exception &e) {
        std::cout << e.what() << std::endl;
    }
}

TEST(Fetch_Coroutine_Test, Fetch) {
    TimerContext context;
    SocketContext socketContext;

    link_socket_to_timer_context(context, socketContext, std::chrono::milliseconds(100));

    fetch_test(socketContext);

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
