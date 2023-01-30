#include "timer_context.h"
#include <winsock2.h>

void TimerContext::check_timers() {
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

long long TimerContext::step() {
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
    Sleep(current_interval_ms);

    check_timers();

    return current_interval_ms;
}

void TimerContext::start() { start_time = current_time_ms(); }

void TimerContext::run() {
    start();
    while (!isExiting) {
        step();
    }
}

long long TimerContext::correct_poll_time(long long current_time) {
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
