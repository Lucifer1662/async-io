#include <asyncio/socket/WSAStartup.h>
#include <asyncio/buffer/CStringBuffer.h>
#include <asyncio/context.h>
#include <asyncio/socket/socket.h>
#include <asyncio/ContextSocket.h>
#include <gtest/gtest.h>

#include "test.h"

// Demonstrate some basic assertions.
TEST(ContextSocketTest, CrissCrossSenderReceiverTest) {
    AsyncioGlobal startup;
    startup.start();
    auto sendBuffer = std::make_unique<CStringBufferSender>("hey");
    auto recvBuffer = std::make_unique<CStringBufferReceiver>();

    bool send_called = false;
    bool send1_called = false;
    bool recv_called = false;
    bool recv1_called = false;

    Context context;
    std::list<ContextSocket> sockets;

    auto server = ContextListeningSocket(context, [&](ContextSocket &&sock) {
        sockets.emplace_back(sock);
        auto &socket = sockets.back();

        socket.read(std::make_unique<CStringBufferReceiver>(),
                    [&](std::unique_ptr<CStringBufferReceiver> buffer, bool error) {
                        recv_called = true;
                        ASSERT_EQ(buffer->getString(), "hey");
                        ASSERT_EQ(error, false);

                        socket.write(std::make_unique<CStringBufferSender>("hello"), [&](auto buffer, bool error) {
                            send1_called = true;
                            ASSERT_EQ(buffer->payload_sent_so_far(), 6);
                            ASSERT_EQ(error, false);
                        });
                    });
    });

    auto server_good = server.start_listening(1235);
    ASSERT_TRUE(server_good);

    auto socket = ContextSocket(context);
    auto address = IPAddress::from_string("localhost", 1235);
    auto connected_successful = socket.connect(*address, [&](auto &socket, bool error) {
        socket.write(std::move(sendBuffer), [&](std::unique_ptr<CStringBufferSender> buffer_ptr, bool error) {
            send_called = true;
            ASSERT_EQ(buffer_ptr->payload_sent_so_far(), 4);
            ASSERT_EQ(error, false);

            socket.read(std::make_unique<CStringBufferReceiver>(),
                        [&](std::unique_ptr<CStringBufferReceiver> buffer, bool error) {
                            recv1_called = true;
                            ASSERT_EQ(buffer->getString(), "hello");
                            ASSERT_EQ(error, false);
                        });
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

    ASSERT_EQ(send_called, true);
    ASSERT_EQ(send1_called, true);
    ASSERT_EQ(recv_called, true);
    ASSERT_EQ(recv1_called, true);
}