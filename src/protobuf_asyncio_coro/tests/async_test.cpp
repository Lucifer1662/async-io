#include <gtest/gtest.h>

#include <iostream>
#include "example.pb.h"
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

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

template <typename ProtoBuf, typename AsyncReadOperation>
AsyncTask<> send_proto_buffer(ProtoBuf &buf, int fd, AsyncReadOperation &write_operation, std::string &buffer) {
    // google::protobuf::io::FileOutputStream fs(fd, 0);
    // OS::non_blocking_socket(fd);

    // while (!buf.SerializeToZeroCopyStream(&fs)) {
    //     std::cout << "Error: " << fs.GetErrno() << std::endl;
    //     co_await write_operation.wait();
    //     std::cout << "Resumed" << std::endl;
    // }

    auto size = buf.ByteSize();
    if (buffer.capacity() < size) {
        buffer.clear();
        buffer.resize(size);
    }
    buf.SerializeToArray(buffer.data(), size);

    co_await write_async_size_t(size, write_operation);
    co_await write_operation(buffer.data(), size);
}

template <typename ProtoBuf, typename AsyncReadOperation>
AsyncTask<> send_proto_buffer(ProtoBuf &buf, int fd, AsyncReadOperation &write_operation) {
    std::string s;
    return send_proto_buffer(buf, fd, write_operation, s);
}

template <typename ProtoBuf> AsyncTask<> send_proto_buffer(ProtoBuf &buf, Socket &socket) {
    co_await send_proto_buffer(buf, socket.FD(), socket.write_operation);
}

template <typename ProtoBuf, typename AsyncWriteOperation>
AsyncTask<> read_proto_buffer(ProtoBuf &buf, int fd, AsyncWriteOperation &read_operation, std::string &buffer) {
    // google::protobuf::io::FileInputStream fs(fd, 0);
    // while (!buf.ParseFromZeroCopyStream(&fs)) {
    //     co_await read_operation.wait();
    // }

    auto size = co_await read_async_size_t(read_operation);
    if (buffer.capacity() < size) {
        buffer.clear();
        buffer.resize(size);
    }
    buf.ParseFromArray(buffer.data(), size);

    co_await read_operation(buffer.data(), size);
}

template <typename ProtoBuf, typename AsyncReadOperation>
AsyncTask<> read_proto_buffer(ProtoBuf &buf, int fd, AsyncReadOperation &read_operation) {
    std::string s;
    return read_proto_buffer(buf, fd, read_operation, s);
}

template <typename ProtoBuf> AsyncTask<> read_proto_buffer(ProtoBuf &buf, Socket &socket) {
    co_await read_proto_buffer(buf, socket.FD(), socket.read_operation);
}

AsyncTask<> handle_new_connection(Socket socket) {
    TestProto::Person person;
    std::string massive;
    // massive.assign(2564096, 'a');
    massive.assign(3000000, 'a');
    person.set_email(massive);
    person.set_id(55);
    person.set_name("example person");

    co_await send_proto_buffer(person, socket);
}

AsyncTask<> accept_connection(ListeningSocket &listening_socket) {
    for (;;) {
        Socket new_connection = co_await listening_socket.accept();
        std::cout << "New Connection" << std::endl;
        handle_new_connection(std::move(new_connection));
    };
}

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

            std::cout << "Connected" << std::endl;

            TestProto::Person person;
            co_await read_proto_buffer(person, socket);

            std::cout << "Name: " << person.name() << std::endl;
            std::cout << "Email Size: " << person.email().size() << std::endl;
            // std::cout << "Email: " << person.email() << std::endl;
            std::cout << "Id: " << person.id() << std::endl;
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
}
