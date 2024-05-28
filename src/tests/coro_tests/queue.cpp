#include <gtest/gtest.h>
#include <coro/queue.h>

TEST(Queue, Simple) {
    Queue<int> mQueue{2};
    auto func1Coro = [&]() -> AsyncTask<> {
        co_await mQueue.Put(5);
        co_await mQueue.Put(4);
        EXPECT_EQ(co_await mQueue.Get(), 5);
        EXPECT_EQ(co_await mQueue.Get(), 4);
        EXPECT_EQ(co_await mQueue.Get(), 3);   // waits
        co_await mQueue.Put(2);
    };
    auto task1 = func1Coro();

    auto func2Coro = [&]() -> AsyncTask<> {
        co_await mQueue.Put(3);
        co_await mQueue.Put(1);
        EXPECT_EQ(co_await mQueue.Get(), 2);
        EXPECT_EQ(co_await mQueue.Get(), 1);
    };
    auto task2 = func2Coro();
}

TEST(Queue, ProducerConsumer) {
    Queue<int> mQueue{2};
    auto func1Coro = [&]() -> AsyncTask<> {
        for (int i = 0; i < 10000; i++) {
            co_await mQueue.Put(i);
        }
    };
    auto task1 = func1Coro();

    auto func2Coro = [&]() -> AsyncTask<> {
        for (int i = 0; i < 10000; i++) {
            EXPECT_EQ(co_await mQueue.Get(), i);
        }
    };
    auto task2 = func2Coro();
}
