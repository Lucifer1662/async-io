#pragma once
#include <asyncio/socket/async_operation.h>

struct TestWriteOperation {
    std::iostream &stream;
    TestWriteOperation(std::iostream &stream) : stream(stream) {}

    int operator()(char *d, int n) {
        for (size_t i = 0; i < n; i++) {
            stream.put(d[i]);
        }
        return n;
    }
};

struct TestReadOperation {
    std::istream &stream;
    TestReadOperation(std::istream &stream) : stream(stream) {}

    int operator()(char *d, int n) {
        auto end = d + n;
        int i = 0;
        for (; d != end && stream.rdbuf()->in_avail() > 0; d++) {
            auto ii = stream.rdbuf()->in_avail();
            *d = stream.get();
            i++;
        }

        return i;
    }
};

using AsyncTestWriteOperation = AsyncOperation<TestWriteOperation>;
using AsyncTestReadOperation = AsyncOperation<TestReadOperation>;
