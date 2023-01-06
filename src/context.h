#pragma once

#include <winsock2.h>
#include <chrono>
#include <vector>
#include <functional>

struct Socket;

struct Flag
{
    int flag;
    Flag(int flag)
        : flag(flag) {}

    bool isRead() { return flag & POLLRDNORM; }
    bool isWrite() { return flag & POLLWRNORM; }
    bool isError() { return flag & POLLNVAL; }

};

class Context
{
    bool isExiting = false;
    std::chrono::milliseconds interval = std::chrono::milliseconds(100);
    std::vector<WSAPOLLFD> socket_events;
    std::vector<std::function<void(Flag)>> sockets;

public:
    template <typename Func>
    void add(int fd, Func &&callback)
    {
        WSAPOLLFD d = {0};
        d.fd = fd;
        d.events = POLLRDNORM | POLLWRNORM;
        socket_events.push_back(d);
        sockets.push_back(std::forward<Func>(callback));
    }

    bool poll()
    {
        auto ms = interval.count();
        int ret = WSAPoll(&socket_events.front(), socket_events.size(), ms);
        if (SOCKET_ERROR == ret)
        {
            return false;
        }

        if (ret)
        {
            auto size = socket_events.size();
            for (size_t i = 0; i < size; i++)
            {
                sockets[i](socket_events[i].revents);
            }
        }
    }

    void step()
    {
        auto start_time = std::chrono::system_clock::now();
        poll();
        auto end_time = std::chrono::system_clock::now();
        auto poll_duration = end_time - start_time;
        // sleep for, use interval
    }

    void run()
    {
        while (!isExiting)
        {
            step();
        }
    }
};
