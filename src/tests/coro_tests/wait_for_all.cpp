#include <gtest/gtest.h>
#include <coro/async_task.h>
#include <coro/event.h>
#include <coro/wait_for_all.h>

auto ExampleCoroutine = [](Event *event = nullptr) -> AsyncTask<int> {
    if (event) {
        co_await event->Wait();
    }

    co_return 3;
};

TEST(WaitForAllTests, Vector_AllReturnImmediately) {

    bool isDone = false;

    auto funcCoro = [&]() -> AsyncTask<> {
        std::vector<AsyncTask<int>> tasks;

        for (size_t i = 0; i < 3; i++) {
            tasks.emplace_back(ExampleCoroutine());
        }

        const auto &result = co_await WaitForAll(tasks);

        isDone = true;
        std::vector<int> expected = {3, 3, 3};
        EXPECT_EQ(result, expected);
    };

    funcCoro();
    EXPECT_TRUE(isDone);
}

TEST(WaitForAllTests, Vector_AllWait) {

    bool isDone = false;
    std::vector<Event> events(3);

    auto funcCoro = [&]() -> AsyncTask<> {
        std::vector<AsyncTask<int>> tasks;

        for (size_t i = 0; i < 3; i++) {
            tasks.emplace_back(ExampleCoroutine(&events[i]));
        }

        const auto &result = co_await WaitForAll(tasks);

        isDone = true;
        std::vector<int> expected = {3, 3, 3};
        EXPECT_EQ(result, expected);
    };

    auto task = funcCoro();
    EXPECT_FALSE(isDone);
    events[0].Fire();
    events[1].Fire();
    events[2].Fire();
    EXPECT_TRUE(isDone);
}

TEST(WaitForAllTests, Iterator_AllReturnImmediately) {

    bool isDone = false;

    auto funcCoro = [&]() -> AsyncTask<> {
        std::vector<AsyncTask<int>> tasks;

        for (size_t i = 0; i < 3; i++) {
            tasks.emplace_back(ExampleCoroutine());
        }
        std::vector<int> result;
        co_await WaitForAll(tasks.begin(), tasks.end(), std::insert_iterator(result, result.end()));

        isDone = true;
        std::vector<int> expected = {3, 3, 3};
        EXPECT_EQ(result, expected);
    };

    funcCoro();
    EXPECT_TRUE(isDone);
}
