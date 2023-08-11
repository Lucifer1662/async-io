#pragma once
#include <vector>
#include "async_task.h"

// template <typename... Args> static AsyncTask<std::vector<ReturnValue>> all_vec(Args &&...tasks) {
//     return std::vector<ReturnValue>({co_await tasks...});
// }

// template <typename... Args> static AsyncTask<std::vector<ReturnValue>> all_tup(Args &&...tasks) {
//     return std::make_tuple(co_await tasks...);
// }

// template <typename... Args> static AsyncTask<void> when_all_done(Args &&...tasks) {
//     std::make_tuple(co_await tasks...);
// }

template <typename ReturnValue>
AsyncTask<std::vector<ReturnValue>> WaitForAll(std::vector<AsyncTask<ReturnValue>> &tasks) {
    // run all tasks
    for (auto &task : tasks) {
        task.RunDetached();
    }

    // wait for all tasks
    std::vector<ReturnValue> results;
    results.reserve(tasks.size());
    for (auto &task : tasks) {
        results.emplace_back(co_await task);
    }

    co_return results;
}