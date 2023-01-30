#include <asyncio/socket/WSAStartup.h>
#include <asyncio/buffer/CStringBuffer.h>
#include <asyncio/context.h>
#include <asyncio/socket/socket.h>
#include <asyncio/ContextSocket.h>
#include <asyncio/lifetime/heart_beat.h>
#include <gtest/gtest.h>

#include "test.h"

template <typename Func> void client_heart_beat(Socket &socket, Func succeeded) {
    socket.write(std::move(std::make_unique<CStringBufferSender>("ping")),
                 [&, succeeded](std::unique_ptr<CStringBufferSender> buffer_ptr, bool error) {
                     if (error)
                         return;
                     socket.read(std::make_unique<CStringBufferReceiver>(),
                                 [&, succeeded](std::unique_ptr<CStringBufferReceiver> buffer, bool error) {
                                     succeeded(buffer->getString() == "pong");
                                 });
                 });
}

// Demonstrate some basic assertions.
TEST(HeartBeatTest, HeartBeatSucceeds) {
    AsyncioGlobal startup;
    startup.start();

    Context context;
    std::list<ContextSocket> sockets;

    auto server = ContextListeningSocket(context, [&](ContextSocket &&sock) {
        sockets.emplace_back(sock);
        auto &socket = sockets.back();

        socket.read(
            std::make_unique<CStringBufferReceiver>(), [&](std::unique_ptr<CStringBufferReceiver> buffer, bool error) {
                if (buffer->getString() == "ping") {
                    // do heart beat
                    socket.write(std::make_unique<CStringBufferSender>("pong"), [&](auto buffer, bool error) {});
                }
            });
    });

    auto server_good = server.start_listening(1235);
    ASSERT_TRUE(server_good);

    auto socket = ContextSocket(context);
    auto address = IPAddress::from_string("localhost", 1235);
    auto connected_successful = socket.connect(*address, [&](Socket &socket, bool error) {
        make_heart_beat(
            context, 1000, [&](auto succeeded) { client_heart_beat(socket, succeeded); },
            []() {
                // should not fail
                ASSERT_TRUE(false);
            });
    });

    ASSERT_TRUE(connected_successful);

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