#pragma once
#include "timer_context.h"
#include <memory>
#include <type_traits>

template <typename Func> struct Interval {
    Func callback;
    TimerContext &context;
    long long ms;
    long long start_time;
    long long step = 0;
    using Return_Type = decltype(callback());
    std::weak_ptr<Interval<Func>> self;

    Interval(Func &&callback, TimerContext &context, long long ms, long long start_time)
        : callback(std::forward<Func>(callback))
        , context(context)
        , ms(ms)
        , start_time(start_time) {}

    void process() {
        step++;
        auto next_interval = step * ms + start_time;

        if constexpr (std::is_same<Return_Type, bool>::value) {
            auto should_continue = callback();
            if (should_continue) {
                renew_timer(next_interval);
            }
        } else {
            renew_timer(next_interval);
            callback();
        }
    }

    void renew_timer(long long time) {
        context.add_timer(time, [self = self.lock()]() { self->process(); });
    }
};

// func should have the signature bool() or void(), return true to continue intervals, false to stop, or void for
// always
template <typename Func> void make_interval(TimerContext &context, long long ms, Func &&func) {
    make_interval_at(context, ms, current_time_ms() + ms, std::forward<Func>(func));
}

// func should have the signature bool() or void(), return true to continue intervals, false to stop, or void for
// always first call will be at start_time
template <typename Func>
void make_interval_at(TimerContext &context, long long ms, long long start_time, Func &&callback) {
    auto interval_object = std::make_shared<Interval<Func>>(std::forward<Func>(callback), context, ms, start_time);
    interval_object->self = interval_object;
    interval_object->renew_timer(start_time);
}