#pragma once
#include <chrono>

long long current_time_ms();
template <typename TimePoint> long long high_res_time_point_to_ms(const TimePoint &time_point) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(time_point.time_since_epoch()).count();
}

long long wait_for_seconds(long long seconds);
long long wait_for_milliseconds(long long milliseconds);
