#pragma once
#include <vector>
#include "async_task.h"

template <typename It1, typename It2> AsyncTask<void> WaitForAll(It1 begin, It1 end, It2 destination) {
    for (; begin != end; begin++) {
        auto result = co_await * begin;
        *destination = std::move(result);
    }
    co_return;
}

template <typename ReturnValue>
AsyncTask<std::vector<ReturnValue>> WaitForAll(std::vector<AsyncTask<ReturnValue>> &tasks) {
    // wait for all tasks
    std::vector<ReturnValue> results;
    results.reserve(tasks.size());
    co_await WaitForAll(tasks.begin(), tasks.end(), std::insert_iterator(results, results.end()));
    co_return results;
}