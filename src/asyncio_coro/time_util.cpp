#include "time_util.h"
#include <chrono>

long long current_time_ms() {
    auto now = std::chrono::high_resolution_clock::now();
    return high_res_time_point_to_ms(now);
}

long long wait_for_seconds(long long seconds) { return current_time_ms() + seconds * 1000; }

long long wait_for_milliseconds(long long milliseconds) { return current_time_ms() + milliseconds; }
