#include <gtest/gtest.h>

#include <iostream>
#include "example.pb.h"

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
#include <protobuf_asyncio_coro.h>

AsyncTask<> handle_new_connection(Socket socket) {
    TestProto::Person person;
    std::string massive;
    massive.assign(300000, 'a');
    person.set_email(massive);
    person.set_id(55);
    person.set_name("example person");
    co_await send_proto_buffer(person, socket);
}

AsyncTask<> accept_connection(ListeningSocket &listening_socket) {
    for (;;) {
        Socket new_connection = co_await listening_socket.accept();
        handle_new_connection(std::move(new_connection));
    };
}

void AssertCorrectPerson(TestProto::Person &person) {
    ASSERT_EQ(person.name(), "example person");
    ASSERT_EQ(person.email().size(), 300000);
    ASSERT_EQ(person.id(), 55);
}

bool finished = false;

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

            TestProto::Person person;
            co_await read_proto_buffer(person, socket);

            AssertCorrectPerson(person);
            finished = true;
        }
    }
}

TEST(Protobuf_Async_Test, Async_Test) {
    TimerContext context;
    SocketContext socketContext;

    link_socket_to_timer_context(context, socketContext, std::chrono::milliseconds(100));

    AsyncTask<> task = socket_read_task(socketContext);

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

    ASSERT_TRUE(finished);
}
