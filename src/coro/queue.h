#pragma once
#include "async_task.h"
#include "event.h"
#include <deque>

template <typename T, typename Collection = std::deque<T>> struct Queue {
    Collection mCollection;
    Event mItemAvailable;
    Event mSpaceAvailable;
    Queue(size_t capacity)
        : mSpaceAvailable(capacity) {}

    template <typename Type> AsyncTask<void> Put(Type &&t) {
        co_await mSpaceAvailable.Wait();

        mCollection.push_front(std::forward<T>(t));

        mItemAvailable.Fire();
        co_return;
    }

    AsyncTask<T> Get() {
        co_await mItemAvailable.Wait();

        auto item = std::move(mCollection.back());
        mCollection.pop_back();

        mSpaceAvailable.Fire();
        co_return std::move(item);
    }
};