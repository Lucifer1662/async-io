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

struct Lock {
    bool mLocked;
    std::deque<Event> mQueue;

    AsyncTask<void> Acquire() {
        if(mLocked){
            mQueue.push_back(Event());
            co_await mQueue.back().Wait();
        }
        mLocked = true;
        co_return;
    }

    AsyncTask<void> Release() {
        mLocked = false;
        while (mQueue.size() > 0) {
            // pop mCoroutines as we resume immediately 
            // which could cause another Acquire() co_await
            auto event = mQueue.front();
            mQueue.pop_front();

            // call next task waiting to acquire lock
            // check if the task is still alive as well
            if (event.Fire()){
                break;
            }
        }
    }
};

template <typename T, typename Collection = std::deque<T>> 
struct MultiQueue {
    Queue mQueue;
    Lock mPutLock;
    Lock mGetLock;
    MultiQueue(size_t capacity)
        : mQueue(capacity) {}

    template <typename Type> AsyncTask<void> 
    Put(Type &&t) {
        co_await mPutLock.Acquire();
        co_await mQueue.Put(std::forward<Type>(t));
        mPutLock.Release();
        co_return;
    }

    AsyncTask<T> Get() {
        co_await mGetLock.Acquire();
        auto item = co_await mQueue.Get();
        mGetLock.Release();
        co_return std::move(item);
    }
};