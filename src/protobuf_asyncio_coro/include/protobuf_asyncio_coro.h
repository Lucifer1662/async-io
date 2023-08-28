#pragma once
#include <cstring>
#include <asyncio_coro/async_task.h>

struct GrowableBuffer {
    char *data;
    size_t size;

    GrowableBuffer()
        : data(nullptr)
        , size(0) {}

    GrowableBuffer(char *data, size_t size)
        : data(data)
        , size(size) {}

    GrowableBuffer(GrowableBuffer &&s)
        : size(s.size)
        , data(s.data) {
        s.data = 0;
        s.size = 0;
    }

    void resize(size_t n) {
        if (n > size) {
            if (data != nullptr)
                free(data);
            data = (char *)malloc(n);
            size = n;
        }
    }

    ~GrowableBuffer() {
        if (size && data) {
            free(data);
        }
    }
};

template <typename ProtoBuf, typename AsyncReadOperation>
AsyncTask<> send_proto_buffer(ProtoBuf &buf, int fd, AsyncReadOperation &write_operation, GrowableBuffer &buffer) {
    auto size = buf.ByteSizeLong();
    buffer.resize(size);
    buf.SerializeToArray(buffer.data, size);

    co_await write_async_size_t(size, write_operation);
    co_await write_operation(buffer.data, size);
}

template <typename ProtoBuf, typename AsyncReadOperation>
AsyncTask<> send_proto_buffer(ProtoBuf &buf, int fd, AsyncReadOperation &write_operation) {
    GrowableBuffer s;
    co_return co_await send_proto_buffer(buf, fd, write_operation, s);
}

template <typename ProtoBuf> AsyncTask<> send_proto_buffer(ProtoBuf &buf, Socket &socket) {
    co_await send_proto_buffer(buf, socket.FD(), socket.write_operation);
}

template <typename ProtoBuf, typename AsyncWriteOperation>
AsyncTask<bool> read_proto_buffer(ProtoBuf &buf, int fd, AsyncWriteOperation &read_operation, GrowableBuffer &buffer) {
    auto size = co_await read_async_size_t(read_operation);
    buffer.resize(size);
    co_await read_operation(buffer.data, size);
    bool success = buf.ParseFromArray(buffer.data, size);
}

template <typename ProtoBuf, typename AsyncReadOperation>
AsyncTask<bool> read_proto_buffer(ProtoBuf &buf, int fd, AsyncReadOperation &read_operation) {
    GrowableBuffer s;
    co_return co_await read_proto_buffer(buf, fd, read_operation, s);
}

template <typename ProtoBuf> AsyncTask<bool> read_proto_buffer(ProtoBuf &buf, Socket &socket) {
    co_return co_await read_proto_buffer(buf, socket.FD(), socket.read_operation);
}
