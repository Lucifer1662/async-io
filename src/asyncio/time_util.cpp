#include "time_util.h"
#include <chrono>

long long current_time_ms() {
    return high_res_time_point_to_ms(std::chrono::high_resolution_clock::now());
}

long long high_res_time_point_to_ms(
    const std::chrono::steady_clock::time_point &time_point) {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               time_point.time_since_epoch())
        .count();
}

long long wait_for_seconds(long long seconds) {
    return current_time_ms() + seconds * 1000;
}

long long wait_for_milliseconds(long long milliseconds) {
    return current_time_ms() + milliseconds;
}
