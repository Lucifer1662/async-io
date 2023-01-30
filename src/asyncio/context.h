#pragma once
#include <stdint.h>

#include <winsock2.h>
#include <chrono>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>
#include "time_util.h"
#include <iostream>

struct Flag {
    int flag;
    Flag(int flag = 0)
        : flag(flag) {}

    bool isRead() const { return flag & POLLRDNORM; }
    bool isWrite() const { return flag & POLLWRNORM; }
    bool isError() const { return flag & POLLNVAL; }
    void setRead() { flag |= POLLRDNORM; }
    void setWrite() { flag |= POLLWRNORM; }
    void setReadIf(bool condition) {
        if (condition)
            setRead();
    }
    void setWriteIf(bool condition) {
        if (condition)
            setWrite();
    }
};

long long wait_time(long long interval, long long computation_duration, long long next_timer, long long current_time);

class Context {
    bool isExiting = false;
    std::chrono::milliseconds interval = std::chrono::milliseconds(100);
    long long start_time = current_time_ms();
    long long computation_cost = 0;
    long long current_interval_ms;
    long long step_count = 0;

    std::vector<WSAPOLLFD> socket_events;
    std::vector<std::function<void(Flag, Context &context, int i)>> sockets;

    using Timers = std::multimap<long long, std::function<void()>>;
    Timers timers;

  private:
    auto cancel_timer(Timers::iterator &it) {

        return [first_time = true, it, this]() mutable {
            if (first_time) {
                this->timers.erase(it);
                first_time = false;
            }
        };
    }

  public:
    Context(std::chrono::milliseconds interval = std::chrono::milliseconds(100))
        : interval(interval)
        , current_interval_ms(interval.count()) {}

    template <typename Func> void add_socket(SOCKET fd, Func &&callback) {
        WSAPOLLFD d = {0};
        d.fd = fd;
        d.events = POLLRDNORM | POLLWRNORM;
        socket_events.push_back(d);
        sockets.push_back(std::forward<Func>(callback));
    }

    void remove_socket(SOCKET fd) {
        auto it = std::find_if(socket_events.begin(), socket_events.end(), [=](WSAPOLLFD e) { return e.fd == fd; });
        if (it != socket_events.end()) {
            // get corresponding index
            auto i = std::distance(socket_events.begin(), it);

            // delete socket event
            *it = socket_events.back();
            socket_events.pop_back();

            // delete socket callback
            sockets[i] = sockets.back();
            sockets.pop_back();
        }
    }

    template <typename Func> auto add_timer(long long posx_time_ms, Func &&func) {
        auto it = timers.insert({posx_time_ms, std::forward<Func>(func)});

        // cancel timer function
        return cancel_timer(it);
    }

    bool poll(int wait_for_ms) {

        auto start_time = current_time_ms();
        bool ret;

        if (socket_events.empty()) {
            Sleep(wait_for_ms);
            ret = false;
        } else {
            ret = SOCKET_ERROR != WSAPoll(&socket_events.front(), socket_events.size(), wait_for_ms);
        }

        auto end_time = current_time_ms();
        auto difference = end_time - start_time;

        if (wait_for_ms > difference) {
            Sleep(wait_for_ms - difference);
        }

        end_time = current_time_ms();
        return ret;
    }

    void check_sockets() {
        auto size = socket_events.size();
        for (size_t i = 0; i < size; i++) {
            sockets[i](socket_events[i].revents, *this, i);
        }
    }

    void check_timers() {

        auto end_time_ms = current_time_ms();
        auto num_timers_at_start = timers.size();
        size_t timers_cleared = 0;
        while (!timers.empty() && end_time_ms >= timers.begin()->first) {
            timers.begin()->second();

            // remove timer from list
            timers.erase(timers.begin());
            timers_cleared++;
            // break early to force re-polling
            if (timers_cleared == num_timers_at_start) {
                break;
            }
        }
    }

    void set_subscription(int socket_i, Flag flag) { socket_events[socket_i].events = flag.flag; }

    long long step();

    // records start time
    // if call step manually must call this function first
    void start() { start_time = current_time_ms(); }

    void run() {
        start();
        while (!isExiting) {
            step();
        }
    }

  private:
    long long correct_poll_time(long long current_time) {
        auto elapse_time = current_time - start_time;
        auto current_step = (elapse_time / interval.count()) + 1;

        if (step_count + 1 < current_step) {
            // skipped steps
            // 1 4
            step_count = current_step - 1;
        } else {
            // 1 2
            step_count = current_step;
        }

        return start_time + step_count * interval.count();
    }
};
