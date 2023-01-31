#include <asyncio/http/http_request_buffer.h>
#include <gtest/gtest.h>
#include "test.h"

const std::string example_get =
    "POST / HTTP/1.1\r\nHost: localhost:1235\r\nUser-Agent: curl/7.83.1\r\nAccept: "
    "*/*\r\nContent-Length: 11\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\nhello world";

const std::string example_get_header = "Host: localhost:1235\r\nUser-Agent: curl/7.83.1\r\nAccept: "
                                       "*/*\r\nContent-Length: 11\r\nContent-Type: application/x-www-form-urlencoded";

const std::string example_body = "hello world";

std::unordered_map<std::string, std::string> expected_header = {{"Host", "localhost:1235"},
                                                                {"User-Agent", "curl/7.83.1"},
                                                                {"Accept", "*/*"},
                                                                {"Content-Length", "11"},
                                                                {"Content-Type", "application/x-www-form-urlencoded"}};

// Demonstrate some basic assertions.
TEST(Http_Util, header_string_matches) { ASSERT_EQ(example_get_header, header_string(example_get)); }

TEST(Http_Util, trim_white_space_works) { ASSERT_EQ("hello world", trim_white_space("  \r\t\nhello world\r\n\t ")); }

template <typename H1, typename H2> void ASSERT_HEADER_EQ(H1 &header, H2 &expected_header) {
    ASSERT_EQ(expected_header.size(), header.size());
    for (auto &[key, value] : expected_header) {
        auto it = header.find(key);
        ASSERT_NE(it, header.end());
        ASSERT_EQ(key, it->first);
        ASSERT_EQ(value, it->second);
    }
}

TEST(Http_Util, get_header_from_string_matchers_header) {
    auto header = get_header_from_string(example_get_header);
    ASSERT_HEADER_EQ(header, expected_header);
}

TEST(HttpBufferReceiverTest, CorrectlyParsesExample1) {

    std::stringstream ss(example_get);
    auto sender = AsyncTestWriteOperation(ss);
    auto receiver = AsyncTestReadOperation(ss);

    bool recv_called = false;
    receiver.request(std::make_unique<HttpBufferReceiver>(),
                     [&](std::unique_ptr<HttpBufferReceiver> buffer_ptr, bool error) {
                         recv_called = true;

                         ASSERT_EQ(buffer_ptr->getBody(), example_body);
                         ASSERT_HEADER_EQ(buffer_ptr->getHeader(), expected_header);
                         ASSERT_EQ(error, false);
                     });

    while (!receiver.check_requests()) {
    }

    ASSERT_EQ(recv_called, true);
}
