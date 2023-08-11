#include <gtest/gtest.h>
#include <coro/async_task.h>
#include <coro/event.h>
#include <coro/wait_for_all.h>

auto ExampleCoroutine = [](Event *event = nullptr) -> AsyncTask<int> {
    if (event) {
        co_await event->Wait();
    }

    co_return 5;
};

TEST(WaitForAll, AllReturnImmediately) {

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

    funcCoro().RunDetached();
    EXPECT_TRUE(isDone);
}
