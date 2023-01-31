#pragma once
#include <chrono>

long long current_time_ms();
long long high_res_time_point_to_ms(const std::chrono::steady_clock::time_point &time_point);
long long wait_for_seconds(long long seconds);
long long wait_for_milliseconds(long long milliseconds);
