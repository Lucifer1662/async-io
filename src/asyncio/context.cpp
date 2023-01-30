#include "context.h"
#include <iostream>

long long wait_time(long long interval, long long computation_duration, long long next_timer, long long current_time) {
    computation_duration -= interval;

    auto initial_comp_duration = computation_duration;

    long long difference = 0;

    auto ms = interval;

    // if first timer is overdue, poll immediately
    if (next_timer < current_time) {
        difference = interval;
    } else {
        // pick smallest time between next timer or polling interval

        // 1400
        // 1000
        // 400
        auto timer_difference = next_timer - current_time;
        if (timer_difference < interval) {
            ms = timer_difference;
            computation_duration -= interval - timer_difference;
        }
    }

    // make sure computation time is positive
    if (computation_duration < 0)
        computation_duration = 0;

    if (computation_duration > ms) {
        computation_duration -= ms;
        ms = 0;
        // cannot poll fast enough to guarantee polling time
        // cause by user work taking too longer than polling interval
    } else {
        // reduce polling time by computation time used up
        ms -= computation_duration;
        computation_duration = 0;
    }

    return ms;
}

long long Context::step() {
    // end computation timer
    auto current_time = current_time_ms();
    auto correct_time = correct_poll_time(current_time);

    auto next_timer = timers.empty()   //
        ? LLONG_MAX
        : timers.begin()->first;

    auto current_interval_ms = next_timer <= correct_time   // next timer is before poll interval
        ? next_timer - current_time                         // wait for next timer
        : correct_time - current_time;                      // wait for correct poll time

    current_interval_ms = std::max(0ll, current_interval_ms);

    // poll events
    auto new_events = poll(current_interval_ms);

    // if new events on socket process sockets
    if (new_events)
        check_sockets();

    // check for completed timers
    // but only check the timers already in the queue
    check_timers();

    return current_interval_ms;
}
