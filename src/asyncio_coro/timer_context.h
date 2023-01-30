#pragma once
#include "time_util.h"

#include <chrono>
#include <map>
#include <functional>
#include "coroutine"
#include "async_task.h"

class TimerContext {
    bool isExiting = false;
    std::chrono::milliseconds interval = std::chrono::milliseconds(100);
    long long start_time = current_time_ms();
    long long computation_cost = 0;
    long long current_interval_ms;
    long long step_count = 0;

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
    TimerContext(std::chrono::milliseconds interval = std::chrono::milliseconds(100))
        : interval(interval)
        , current_interval_ms(interval.count()) {}

    template <typename Func> auto add_timer(long long posx_time_ms, Func &&func) {
        auto it = timers.insert({posx_time_ms, std::forward<Func>(func)});

        // cancel timer function
        return cancel_timer(it);
    }

    void check_timers();

    long long step();

    // records start time
    // if call step manually must call this function first
    void start();

    void run();

  private:
    long long correct_poll_time(long long current_time);
};

auto async_wait_until_ms(long long time_ms, TimerContext &timerContext) {
    struct Awaitable {
        long long time_ms;
        TimerContext &timerContext;

        constexpr bool await_ready() const noexcept { return false; }
        void await_resume() const noexcept {}
        void await_suspend(coroutine_handle<AsyncTask<>::promise_type> h) {
            timerContext.add_timer(time_ms, [h]() { h.resume(); });
        }
    };

    return Awaitable{time_ms, timerContext};
}

auto async_wait_for_ms(long long ms, TimerContext &timerContext) {
    return async_wait_until_ms(wait_for_milliseconds(ms), timerContext);
}
