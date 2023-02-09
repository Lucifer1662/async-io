#pragma once
#include "async_task.h"
#include "endian.h"
#include <string>

template <typename Buffer, typename AsyncOperation>
AsyncTask<int> async_buffer_operation(Buffer &b, AsyncOperation &async_operation) {
    int total_consumed = 0;

    for (;;) {
        auto [data, size] = b.contiguous();
        if (size == 0) {
            co_return total_consumed;
        }

        int num_written = async_operation.poll_op(data, size);
        if (num_written == -1) {
            // probably should report error at some point :/
            num_written = 0;
        }
        total_consumed += num_written;
        auto unused = b.advance(num_written);
        if (unused > 0) {
            async_operation.assign_overflow(data + size - unused, unused);
        }

        if (num_written < size) {
            co_await async_operation.wait();
        }
    }
}

template <typename AsyncOperation> AsyncTask<std::string> read_async_c_string(AsyncOperation &read_operation) {
    std::string str;
    char c;
    for (;;) {
        int num_read = read_operation.poll_op(&c, 1);

        if (num_read == 1) {
            if (c == 0) {
                co_return std::move(str);
            } else {
                str += c;
            }
        } else {
            co_await read_operation.wait();
        }
    }
}

template <typename AsyncOperation>
AsyncTask<> write_async_c_string(const std::string &str, AsyncOperation &write_operation) {
    return write_operation(str.c_str(), str.size() + 1);
}

template <typename AsyncOperation> AsyncTask<> write_async_size_t(size_t number, AsyncOperation &write_operation) {
    number = host_to_network64(number);
    co_await write_operation((char *)&number, sizeof(size_t));
}

template <typename AsyncOperation> AsyncTask<size_t> read_async_size_t(AsyncOperation &read_operation) {
    size_t number;
    co_await read_operation((char *)&number, sizeof(size_t));
    co_return network_to_host64(number);
}