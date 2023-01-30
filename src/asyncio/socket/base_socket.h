#pragma once
#include "../buffer/buffer.h"

template <typename AsyncWriteOperation, typename AsyncReadOperation> class BaseSocket {
  protected:
    AsyncWriteOperation writeOps;
    AsyncReadOperation readOps;

  public:
    BaseSocket(AsyncWriteOperation writeOps, AsyncReadOperation readOps)
        : writeOps(std::move(writeOps)), readOps(std::move(readOps)) {}

    BaseSocket(const BaseSocket &s) = delete;
    BaseSocket(BaseSocket &&s) = default;

    template <typename Buffer_T, typename Func> void read(std::unique_ptr<Buffer_T> &&buffer, Func &&func) {
        readOps.request(std::move(buffer), std::forward<Func>(func));
    }
    template <typename Buffer_T, typename Func> void write(std::unique_ptr<Buffer_T> &&buffer, Func &&func) {
        writeOps.request(std::move(buffer), std::forward<Func>(func));
    }

    bool reads_available() { return readOps.check_requests(); }
    bool write_available() { return writeOps.check_requests(); }

    size_t wants_to_read() { return readOps.wants_more(); }
    size_t wants_to_write() { return writeOps.wants_more(); }
};