#pragma once

#include "os_socket.h"
#include "async_operation.h"
#include <optional>
#include "async_task.h"
#include "IpAddress.h"

class RawSocket {
    OS::SOCKET fd;

  public:
    RawSocket(OS::SOCKET fd)
        : fd(fd)
        , read_operation(fd)
        , write_operation(fd) {}
    ReadAsyncOperation read_operation;
    WriteAsyncOperation write_operation;

    auto FD() { return fd; }
    static std::optional<RawSocket> create();

    void read_available();
    void write_available();

    AsyncTask<> connect(const IPAddress &address);

    bool start_listening(int port);

    AsyncTask<std::optional<RawSocket>> accept();

    void destroy_dependent_coroutines();
};

class RawListeningSocket {
    OS::SOCKET fd;
    int port;

    RawListeningSocket(OS::SOCKET fd)
        : fd(fd)
        , accept_operation(fd) {}

  public:
    AcceptAsyncOperation accept_operation;

    auto FD() { return fd; }
    static std::optional<RawListeningSocket> create();

    void accept_available();

    bool start_listening(int port);
    AsyncTask<std::optional<RawSocket>> accept();
    void destroy_dependent_coroutines();
};
