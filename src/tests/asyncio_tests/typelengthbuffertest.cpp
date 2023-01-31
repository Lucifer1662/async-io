#include <gtest/gtest.h>
#include <asyncio/buffer/TypeLengthBuffer.h>
#include "test.h"

TEST(TypeLengthBufferTests, CrissCrossSenderReceiverTest) {
    auto sendBuffer = std::make_unique<TypeLengthBufferSender>(5, std::vector<char>({'h', 'e', 'y'}));
    auto recvBuffer = std::make_unique<TypeLengthBufferReceiver>();

    std::stringstream ss;
    auto sender = AsyncTestWriteOperation(ss);
    auto receiver = AsyncTestReadOperation(ss);

    bool send_called = false;
    sender.request(std::move(sendBuffer), [&](auto buffer_ptr, bool error) {
        send_called = true;
        ASSERT_EQ(buffer_ptr->payload_sent_so_far(), 3);
        ASSERT_EQ(error, false);
    });

    bool recv_called = false;
    receiver.request(std::move(recvBuffer), [&](std::unique_ptr<TypeLengthBufferReceiver> buffer_ptr, bool error) {
        recv_called = true;

        ASSERT_EQ(buffer_ptr->get_size(), 3);
        ASSERT_EQ(buffer_ptr->get_type(), 5);
        ASSERT_EQ(buffer_ptr->view_data(), std::vector<char>({'h', 'e', 'y'}));
        ASSERT_EQ(error, false);
    });

    while (!sender.check_requests()) {
    }

    ss.seekg(0);
    while (!receiver.check_requests()) {
    }

    ASSERT_EQ(send_called, true);
    ASSERT_EQ(recv_called, true);
}
