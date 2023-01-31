#include <gtest/gtest.h>
#include <asyncio/buffer/CStringBuffer.h>
#include "test.h"

TEST(CStringBufferTests, CrissCrossSenderReceiverTest) {
    auto sendBuffer = std::make_unique<CStringBufferSender>("hey");
    auto recvBuffer = std::make_unique<CStringBufferReceiver>();

    std::stringstream ss;
    auto sender = AsyncTestWriteOperation(ss);
    auto receiver = AsyncTestReadOperation(ss);

    bool send_called = false;
    sender.request(std::move(sendBuffer), [&](auto buffer_ptr, bool error) {
        send_called = true;
        // includes zero character at the end of a c string
        ASSERT_EQ(buffer_ptr->payload_sent_so_far(), 4);
        ASSERT_EQ(error, false);
    });

    bool recv_called = false;
    receiver.request(std::move(recvBuffer), [&](std::unique_ptr<CStringBufferReceiver> buffer_ptr, bool error) {
        recv_called = true;
        ASSERT_EQ(buffer_ptr->getString(), "hey");
    });

    while (!sender.check_requests()) {
    }

    ss.seekg(0);
    while (!receiver.check_requests()) {
    }

    ASSERT_EQ(send_called, true);
    ASSERT_EQ(recv_called, true);
}

TEST(CStringBufferTests, CrissCrossSender_BatchReceiverTest_Consume_Sync) {
    auto sendBuffer = std::make_unique<CStringBufferSender>("hey");
    auto send1Buffer = std::make_unique<CStringBufferSender>("hello");
    auto send2Buffer = std::make_unique<CStringBufferSender>("howdy");
    auto recvBuffer = std::make_unique<CStringBatchBufferReceiver>(100);
    auto recv1Buffer = std::make_unique<CStringBatchBufferReceiver>(100);
    auto recv2Buffer = std::make_unique<CStringBatchBufferReceiver>(100);

    std::stringstream ss;
    auto sender = AsyncTestWriteOperation(ss);
    auto receiver = AsyncTestReadOperation(ss);

    bool send_called = false;
    bool send1_called = false;
    bool recv_called = false;
    bool recv1_called = false;
    bool recv2_called = false;

    // send both messages
    sender.request(std::move(sendBuffer), [&](auto buffer_ptr, bool error) {
        send_called = true;
        ASSERT_EQ(buffer_ptr->payload_sent_so_far(), 4);
        ASSERT_EQ(error, false);
    });

    sender.request(std::move(send1Buffer), [&](auto buffer_ptr, bool error) {
        send1_called = true;
        ASSERT_EQ(buffer_ptr->payload_sent_so_far(), 6);
        ASSERT_EQ(error, false);
    });

    sender.request(std::move(send2Buffer), [&](auto buffer_ptr, bool error) {});

    // receive both messages, but the first recvBuffer over reads
    receiver.request(std::move(recvBuffer), [&](auto buffer_ptr, bool error) {
        recv_called = true;
        ASSERT_EQ(buffer_ptr->getString(), "hey");
    });

    receiver.request(std::move(recv1Buffer), [&](auto buffer_ptr, bool error) {
        recv1_called = true;
        ASSERT_EQ(buffer_ptr->getString(), "hello");
    });

    receiver.request(std::move(recv2Buffer), [&](auto buffer_ptr, bool error) {
        recv2_called = true;
        ASSERT_EQ(buffer_ptr->getString(), "howdy");
    });

    while (!sender.check_requests()) {
    }

    ss.seekg(0);
    while (!receiver.check_requests()) {
    }

    ASSERT_EQ(send_called, true);
    ASSERT_EQ(send1_called, true);
    ASSERT_EQ(recv_called, true);
    ASSERT_EQ(recv1_called, true);
    ASSERT_EQ(recv2_called, true);
}

TEST(CStringBufferTests, CrissCrossSender_BatchReceiverTest_Consume_ASync) {
    auto sendBuffer = std::make_unique<CStringBufferSender>("hey");
    auto send1Buffer = std::make_unique<CStringBufferSender>("hello");
    auto send2Buffer = std::make_unique<CStringBufferSender>("howdy");
    auto recvBuffer = std::make_unique<CStringBatchBufferReceiver>(100);
    auto recv1Buffer = std::make_unique<CStringBatchBufferReceiver>(100);
    auto recv2Buffer = std::make_unique<CStringBatchBufferReceiver>(100);

    std::stringstream ss;
    auto sender = AsyncTestWriteOperation(ss);
    auto receiver = AsyncTestReadOperation(ss);

    bool send_called = false;
    bool send1_called = false;
    bool recv_called = false;
    bool recv1_called = false;
    bool recv2_called = false;

    // send both messages
    sender.request(std::move(sendBuffer), [&](auto buffer_ptr, bool error) {
        send_called = true;
        ASSERT_EQ(buffer_ptr->payload_sent_so_far(), 4);
        ASSERT_EQ(error, false);
    });

    sender.request(std::move(send1Buffer), [&](auto buffer_ptr, bool error) {
        send1_called = true;
        ASSERT_EQ(buffer_ptr->payload_sent_so_far(), 6);
        ASSERT_EQ(error, false);
    });

    sender.request(std::move(send2Buffer), [&](auto buffer_ptr, bool error) {});

    // receive both messages, but the first recvBuffer over reads
    receiver.request(std::move(recvBuffer), [&](auto buffer_ptr, bool error) {
        recv_called = true;
        ASSERT_EQ(buffer_ptr->getString(), "hey");
    });

    while (!sender.check_requests()) {
    }

    ss.seekg(0);
    while (!receiver.check_requests()) {
    }

    receiver.request(std::move(recv1Buffer), [&](auto buffer_ptr, bool error) {
        recv1_called = true;
        ASSERT_EQ(buffer_ptr->getString(), "hello");
    });

    receiver.request(std::move(recv2Buffer), [&](auto buffer_ptr, bool error) {
        recv2_called = true;
        ASSERT_EQ(buffer_ptr->getString(), "howdy");
    });

    while (!receiver.check_requests()) {
    }

    ASSERT_EQ(send_called, true);
    ASSERT_EQ(send1_called, true);
    ASSERT_EQ(recv_called, true);
    ASSERT_EQ(recv1_called, true);
    ASSERT_EQ(recv2_called, true);
}
