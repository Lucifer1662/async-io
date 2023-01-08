#pragma once

#include <winsock2.h>
#include <chrono>
#include <vector>
#include <map>
#include <functional>
#include "time.h"

class Socket;

struct Flag
{
    int flag;
    Flag(int flag = 0)
        : flag(flag) {}

    bool isRead() const { return flag & POLLRDNORM; }
    bool isWrite() const { return flag & POLLWRNORM; }
    bool isError() const { return flag & POLLNVAL; }
    void setRead() { flag |= POLLRDNORM; }
    void setWrite() { flag |= POLLWRNORM; }
    void setReadIf(bool condition)
    {
        if (condition)
            setRead();
    }
    void setWriteIf(bool condition)
    {
        if (condition)
            setWrite();
    }
};


class Context
{
    bool isExiting = false;
    std::chrono::milliseconds interval = std::chrono::milliseconds(100);
    int computation_duration = 0;
    std::vector<WSAPOLLFD> socket_events;
    std::vector<std::function<void(Flag, Context &context, int i)>> sockets;

    using Timers = std::multimap<long long, std::function<void()>>;
    Timers timers;


private:

    auto cancel_timer(Timers::iterator& it){

        return [first_time = true, it, this]() mutable {
            if(first_time){
                this->timers.erase(it);
                first_time = false;
            }
        };
    }
  

public:
    template <typename Func>
    void add_socket(SOCKET fd, Func &&callback)
    {
        WSAPOLLFD d = {0};
        d.fd = fd;
        d.events = POLLRDNORM | POLLWRNORM;
        socket_events.push_back(d);
        sockets.push_back(std::forward<Func>(callback));
    }

    template <typename Func>
    auto add_timer(long long posx_time_ms, Func &&func)
    {
        auto it = timers.insert({posx_time_ms, std::forward<Func>(func)});

        //cancel timer function
        return cancel_timer(it);
    }

    

    bool poll(int wait_for_ms)
    {
        int ret = WSAPoll(&socket_events.front(), socket_events.size(), wait_for_ms);
        if (SOCKET_ERROR == ret)
        {
            return false;
        }
        return ret;
    }

    void check_sockets()
    {
        auto size = socket_events.size();
        for (size_t i = 0; i < size; i++)
        {
            sockets[i](socket_events[i].revents, *this, i);
        }
    }

    void check_timers(){
        auto end_time_ms = current_time_ms();
        auto num_timers_at_start = timers.size();
        size_t timers_cleared = 0;
        while (!timers.empty() && end_time_ms >= timers.begin()->first)
        {   
            timers.begin()->second();

            //remove timer from list
            timers.erase(timers.begin());
            timers_cleared++;
            // break early to force re-polling
            if (timers_cleared == num_timers_at_start)
            {
                break;
            }
        }
    }

    void set_subscription(int socket_i, Flag flag)
    {
        socket_events[socket_i].events = flag.flag;
    }

    void step()
    {
        auto start_time = current_time_ms();
        auto ms = interval.count();
        if (computation_duration > ms)
        {
            ms = 0;
            // cannot poll fast enough to guarantee polling time
            // cause by user work taking too longer than polling interval
        }
        else
        {
            //reduce polling time by computation time used up
            ms -= computation_duration;
        }

        if(!timers.empty()){
            //if first timer is overdue, poll immediately 
            if (timers.begin()->first < start_time)
            {
                ms = 0;
            }
            else
            {
                //pick smallest time between next timer or polling interval
                ms = min(timers.begin()->first - start_time, ms);
            }
        }

        //poll events
        auto new_events = poll(ms);
        
        //start computation timer
        start_time = current_time_ms();

        // if new events on socket process sockets
        if (new_events)
            check_sockets();

        // check for completed timers
        // but only check the timers already in the queue
        check_timers();

        //end computation timer
        auto end_time_ms = current_time_ms();
        computation_duration = end_time_ms - start_time;
    }

    void run()
    {
        while (!isExiting)
        {
            step();
        }
    }
};
