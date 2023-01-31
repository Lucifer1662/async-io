#include <gtest/gtest.h>
#include <asyncio/buffer/CStringBuffer.h>
#include <asyncio/socket/base_socket.h>
#include "test.h"

// Demonstrate some basic assertions.
TEST(BaseSocketTest, CrissCrossSenderReceiverTest) {
    auto sendBuffer = std::make_unique<CStringBufferSender>("hey");
    auto recvBuffer = std::make_unique<CStringBufferReceiver>();

    std::stringstream ss;

    auto socket = BaseSocket(AsyncTestWriteOperation(ss), AsyncTestReadOperation(ss));

    bool send_called = false;
    socket.write(std::move(sendBuffer), [&](std::unique_ptr<CStringBufferSender> buffer_ptr, bool error) {
        send_called = true;
        // includes zero character at the end of a c string
        ASSERT_EQ(buffer_ptr->payload_sent_so_far(), 4);
        ASSERT_EQ(error, false);
    });

    bool recv_called = false;
    socket.read(std::move(recvBuffer), [&](std::unique_ptr<CStringBufferReceiver> buffer_ptr, bool error) {
        recv_called = true;
        ASSERT_EQ(buffer_ptr->getString(), "hey");
    });

    while (!socket.write_available()) {
    }

    ss.seekg(0);
    while (!socket.reads_available()) {
    }

    ASSERT_EQ(send_called, true);
    ASSERT_EQ(recv_called, true);
}