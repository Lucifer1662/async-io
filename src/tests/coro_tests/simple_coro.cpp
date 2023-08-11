#include <gtest/gtest.h>
#include <coro/async_task.h>
#include <coro/event.h>

int numberOfFrees = 0;
struct TestNum {
    int n = 5;
    ~TestNum() { numberOfFrees++; }
};

TEST(CoroutineTests, ReturnImmediatelyReference) {
    bool isDone = false;
    auto GetNumberReturnRef = []() -> AsyncTask<std::unique_ptr<int>> {
        auto a = std::make_unique<int>(5);
        co_return a;
    };

    auto funcCoro = [&]() -> AsyncTask<> {
        auto task = GetNumberReturnRef();
        auto num = *(co_await task);
        isDone = true;
        EXPECT_EQ(5, num);
    };
    funcCoro().RunDetached();
    EXPECT_TRUE(isDone);
}

TEST(CoroutineTests, ReturnImmediatelyRValue) {
    bool isDone = false;
    auto GetNumberReturnRValue = []() -> AsyncTask<std::unique_ptr<int>> { co_return std::make_unique<int>(5); };

    auto funcCoro = [&]() -> AsyncTask<> {
        auto task = GetNumberReturnRValue();
        auto num = *(co_await task);
        isDone = true;
        EXPECT_EQ(5, num);
    };
    funcCoro().RunDetached();
    EXPECT_TRUE(isDone);
}

TEST(CoroutineTests, ReturnImmediatelyCopyValue) {
    bool isDone = false;
    auto GetNumberReturnRValue = []() -> AsyncTask<int> { co_return 5; };

    auto funcCoro = [&]() -> AsyncTask<> {
        auto task = GetNumberReturnRValue();
        auto num = co_await task;
        isDone = true;
        EXPECT_EQ(5, num);
    };
    funcCoro().RunDetached();
    EXPECT_TRUE(isDone);
}

TEST(CoroutineTests, ReturnImmediatelyCopyValueStartAndThenWait) {
    numberOfFrees = 0;
    bool isDone = false;
    int hasComputed = 0;
    auto GetNumberReturnRValue = [&]() -> AsyncTask<std::unique_ptr<TestNum>> {
        hasComputed++;
        co_return std::make_unique<TestNum>();
    };

    {
        auto funcCoro = [&]() -> AsyncTask<> {
            auto task = GetNumberReturnRValue();
            task.RunDetached();
            auto num = (co_await task)->n;
            isDone = true;
            EXPECT_EQ(5, num);
        };
        funcCoro().RunDetached();
    }
    EXPECT_TRUE(isDone);
    EXPECT_EQ(hasComputed, 1);
    EXPECT_EQ(numberOfFrees, 1);
}

TEST(CoroutineTests, SuspendedCoroutineIsDestroyedWhenAsyncTaskIsDestroyed) {
    numberOfFrees = 0;
    bool isDone = false;
    int hasComputed = 0;
    Event event;

    auto GetNumberReturnRValue = [&]() -> AsyncTask<std::unique_ptr<TestNum>> {
        hasComputed++;
        auto a = std::make_unique<TestNum>();
        co_await event.Wait();
        co_return a;
    };

    {
        auto funcCoro = [&]() -> AsyncTask<> {
            auto task = GetNumberReturnRValue();
            auto num = (co_await task)->n;
            isDone = true;
            EXPECT_EQ(5, num);
        };
        EXPECT_EQ(numberOfFrees, 0);
        auto task = funcCoro();
        task.RunDetached();
        EXPECT_EQ(numberOfFrees, 0);
    }
    EXPECT_FALSE(isDone);
    EXPECT_EQ(hasComputed, 1);
    EXPECT_EQ(numberOfFrees, 1);
}

TEST(CoroutineTests, LoopReturnValuesImmediately) {
    bool isDone = false;
    auto GetNumberReturnRValue = []() -> AsyncTask<std::unique_ptr<int>> { co_return std::make_unique<int>(5); };

    auto funcCoro = [&]() -> AsyncTask<> {
        int sum = 0;
        for (size_t i = 0; i < 100000; i++) {
            auto task = GetNumberReturnRValue();

            /* code */
            sum += *(co_await task);
        }
        isDone = true;
        EXPECT_EQ(sum, 500000);
    };
    funcCoro().RunDetached();
    EXPECT_TRUE(isDone);
}

TEST(CoroutineTests, LoopReturnValuesAfterWaiting) {
    bool isDone = false;
    Event event;
    int isWaiting = 0;
    int hasWaited = 0;

    auto GetNumberReturnRValue = [&]() -> AsyncTask<std::unique_ptr<int>> {
        isWaiting++;
        co_await event.Wait();
        hasWaited++;
        co_return std::make_unique<int>(5);
    };

    auto funcCoro = [&]() -> AsyncTask<> {
        int sum = 0;
        for (size_t i = 0; i < 100000; i++) {
            auto task = GetNumberReturnRValue();
            sum += *(co_await task);
        }
        isDone = true;
        EXPECT_EQ(sum, 500000);
    };
    auto task = funcCoro();
    task.RunDetached();
    for (size_t i = 0; i < 100000; i++) {
        EXPECT_EQ(isWaiting, i + 1);
        EXPECT_EQ(hasWaited, i);
        event.Fire();
        if (i != 100000 - 1) {
            EXPECT_EQ(isWaiting, i + 2);
        }
        EXPECT_EQ(hasWaited, i + 1);
    }
    EXPECT_TRUE(isDone);
}

TEST(CoroutineTests, WhenReadyReturnImmediately) {
    bool isDone = false;
    auto GetNumberReturnRValue = []() -> AsyncTask<int> { co_return 5; };

    auto funcCoro = [&]() -> AsyncTask<> {
        auto task = GetNumberReturnRValue();
        co_await task.WhenReady();
        isDone = true;
    };
    funcCoro().RunDetached();
    EXPECT_TRUE(isDone);
}

TEST(CoroutineTests, WaitForEventThatFiresButTheCoroutineDied) {
    bool isDone = false;
    bool isWaiting = false;
    bool hasWaited = false;
    Event event;
    auto GetNumberReturnRValue = [&]() -> AsyncTask<std::unique_ptr<int>> {
        isWaiting = true;
        co_await event.Wait();
        hasWaited = true;
        co_return std::make_unique<int>(5);
    };

    auto funcCoro = [&]() -> AsyncTask<> {
        auto task = GetNumberReturnRValue();
        auto num = *(co_await task);
        isDone = true;
        EXPECT_EQ(5, num);
    };
    funcCoro().RunDetached();
    // funcCoro is deleted here

    EXPECT_TRUE(isWaiting);
    EXPECT_FALSE(hasWaited);
    event.Fire();
    EXPECT_FALSE(hasWaited);
    EXPECT_FALSE(isDone);
}

TEST(CoroutineTests, WaitForEventThatFiresAfterWait) {
    bool isDone = false;
    bool isWaiting = false;
    bool hasWaited = false;
    Event event;
    auto GetNumberReturnRValue = [&]() -> AsyncTask<std::unique_ptr<int>> {
        isWaiting = true;
        co_await event.Wait();
        hasWaited = true;
        co_return std::make_unique<int>(5);
    };

    auto funcCoro = [&]() -> AsyncTask<> {
        auto task = GetNumberReturnRValue();
        auto num = *(co_await task);
        isDone = true;
        EXPECT_EQ(5, num);
    };
    auto task = funcCoro();

    task.RunDetached();
    EXPECT_TRUE(isWaiting);
    EXPECT_FALSE(hasWaited);
    event.Fire();
    EXPECT_TRUE(hasWaited);
    EXPECT_TRUE(isDone);
}

TEST(CoroutineTests, WaitForEventThatFireBeforeWait) {
    bool isDone = false;
    bool isWaiting = false;
    Event event;
    event.Fire();
    auto GetNumberReturnRValue = [&]() -> AsyncTask<std::unique_ptr<int>> {
        isWaiting = true;
        co_await event.Wait();
        co_return std::make_unique<int>(5);
    };

    auto funcCoro = [&]() -> AsyncTask<> {
        auto task = GetNumberReturnRValue();
        auto num = *(co_await task);
        isDone = true;
        EXPECT_EQ(5, num);
    };
    funcCoro().RunDetached();
    EXPECT_TRUE(isWaiting);
    EXPECT_TRUE(isDone);
}

TEST(CoroutineTests, WaitForEventThatFireBeforeWaitAndAfter) {
    bool isDone = false;
    int isWaiting = 0;
    Event event;
    event.Fire();
    auto GetNumberReturnRValue = [&]() -> AsyncTask<std::unique_ptr<int>> {
        isWaiting++;
        co_await event.Wait();
        co_return std::make_unique<int>(5);
    };

    auto funcCoro = [&]() -> AsyncTask<> {
        auto task = GetNumberReturnRValue();
        auto num = *(co_await task);
        isDone = true;
        EXPECT_EQ(5, num);
    };
    funcCoro().RunDetached();
    EXPECT_EQ(isWaiting, 1);
    event.Fire();

    EXPECT_EQ(isWaiting, 1);
    EXPECT_TRUE(isDone);
}

TEST(CoroutineTests, WaitForEventThatNeverFires) {
    bool isDone = false;
    bool isWaiting = false;
    bool isFinishedWaiting = false;
    Event event;
    auto GetNumberReturnRValue = [&]() -> AsyncTask<std::unique_ptr<int>> {
        isWaiting = true;
        co_await event.Wait();
        isFinishedWaiting = true;
        co_return std::make_unique<int>(5);
    };

    auto funcCoro = [&]() -> AsyncTask<> {
        auto task = GetNumberReturnRValue();
        auto num = *(co_await task);
        isDone = true;
        EXPECT_EQ(5, num);
    };
    funcCoro().RunDetached();
    EXPECT_TRUE(isWaiting);
    EXPECT_FALSE(isFinishedWaiting);
    EXPECT_FALSE(isDone);
}

TEST(CoroutineTests, WaitForEventButTheEventDies) {
    bool isDone = false;
    bool isWaiting = false;
    bool isFinishedWaiting = false;
    Event *eventPtr;
    auto GetNumberReturnRValue = [&]() -> AsyncTask<std::unique_ptr<int>> {
        isWaiting = true;
        co_await eventPtr->Wait();
        isFinishedWaiting = true;
        co_return std::make_unique<int>(5);
    };

    auto funcCoro = [&]() -> AsyncTask<> {
        auto task = GetNumberReturnRValue();
        auto num = *(co_await task);
        isDone = true;
        EXPECT_EQ(5, num);
    };

    {
        auto task = funcCoro();

        {
            Event event;
            eventPtr = &event;

            task.RunDetached();
        }
    }

    EXPECT_TRUE(isWaiting);
    EXPECT_FALSE(isFinishedWaiting);
    EXPECT_FALSE(isDone);
}
