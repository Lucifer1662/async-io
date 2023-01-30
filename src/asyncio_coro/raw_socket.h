#pragma once

#include <WinSock2.h>
#include "async_operation.h"
#include <optional>
#include "async_task.h"
#include "IpAddress.h"

inline void CLOSESOCK(SOCKET &s) {
    if (INVALID_SOCKET != s) {
        closesocket(s);
        s = INVALID_SOCKET;
    }
}

class RawSocket {
    SOCKET fd;

    RawSocket(SOCKET fd)
        : fd(fd)
        , read_operation(fd)
        , write_operation(fd) {}

  public:
    ReadAsyncOperation read_operation;
    WriteAsyncOperation write_operation;

    auto FD() { return fd; }
    static std::optional<RawSocket> create();

    void read_available();

    void write_available();

    AsyncTask<> connect(const IPAddress &address);

    bool start_listening(int port);

    AsyncTask<std::optional<RawSocket>> accept();
};
