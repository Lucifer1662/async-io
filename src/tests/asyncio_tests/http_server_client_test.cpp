#include <gtest/gtest.h>
#include <asyncio/socket/WSAStartup.h>
#include <asyncio/buffer/CStringBuffer.h>
#include <asyncio/context.h>
#include <asyncio/socket/socket.h>
#include <asyncio/ContextSocket.h>
#include <asyncio/http/fetch.h>
#include <asyncio/http/http_server.h>
#include "test.h"

// Demonstrate some basic assertions.
TEST(HttpServerClient, CrissCrossSenderReceiverTest) {
    bool fetch_returned = false;

    AsyncioGlobal startup;
    startup.start();

    Context context;

    HttpServer server(context, [](auto buffer, auto &socket) {
        std::string response = "HTTP/1.1 200 OK\r\nServer: WebServer\r\nContent-Type: "
                               "text/html\r\nContent-Length: 4\r\nConnection: close\r\n\r\nyeet";

        socket.write(std::make_unique<ContainerBufferSender<std::string>>(std::move(response)),
                     [](auto buffer, bool error) {});
    });

    auto server_is_good = server.start();

    ASSERT_TRUE(server_is_good);

    Fetcher fetcher(context);

    FetchParams params;
    params.url.host = "127.0.0.1";

    fetcher.fetch(std::move(params), [&](std::unique_ptr<HttpBufferReceiver> buffer, bool error) {
        ASSERT_EQ(buffer->getBody(), "yeet");
        fetch_returned = true;
    });

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
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();
    context.step();

    ASSERT_TRUE(fetch_returned);
}