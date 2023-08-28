#include <gtest/gtest.h>
#include <coro/callback_task.h>
#include <coro/event.h>

TEST(Callback, VoidNestedReturnCalledLater) {
    bool handled = false;
    Event event;
    auto funcCoro = [&]() -> AsyncTask<> { co_await event.Wait(); };
    auto coroRunner = [&]() -> CallbackTask<void> {
        co_await funcCoro();
        co_return;
    };
    auto task = coroRunner();
    EXPECT_FALSE(handled);
    task.Handle([&](std::exception_ptr e) { handled = true; });
    EXPECT_FALSE(handled);
    event.Fire();
    EXPECT_TRUE(handled);
}

TEST(Callback, VoidNestedReturnCalledImmediately) {
    bool handled = false;
    Event event;
    auto funcCoro = [&]() -> AsyncTask<> { co_await event.Wait(); };
    auto coroRunner = [&]() -> CallbackTask<void> {
        co_await funcCoro();
        co_return;
    };
    auto task = coroRunner();
    event.Fire();
    EXPECT_FALSE(handled);
    task.Handle([&](std::exception_ptr e) { handled = true; });
    EXPECT_TRUE(handled);
}

TEST(Callback, VoidReturnCalledImmediately) {
    bool handled = false;
    Event event;
    auto coroRunner = [&]() -> CallbackTask<void> {
        co_await event.Wait();
        co_return;
    };
    auto task = coroRunner();
    event.Fire();
    EXPECT_FALSE(handled);
    task.Handle([&](std::exception_ptr e) { handled = true; });
    EXPECT_TRUE(handled);
}

TEST(Callback, VoidReturnCalledAfter) {
    bool handled = false;
    Event event;
    auto coroRunner = [&]() -> CallbackTask<void> {
        co_await event.Wait();
        co_return;
    };
    auto task = coroRunner();
    EXPECT_FALSE(handled);
    task.Handle([&](std::exception_ptr e) { handled = true; });
    EXPECT_FALSE(handled);
    event.Fire();
    EXPECT_TRUE(handled);
}

TEST(Callback, IntNestedReturnCalledLater) {
    bool handled = false;
    Event event;
    auto funcCoro = [&]() -> AsyncTask<> { co_await event.Wait(); };
    auto coroRunner = [&]() -> CallbackTask<int> {
        co_await funcCoro();
        co_return 5;
    };
    auto task = coroRunner();
    EXPECT_FALSE(handled);
    task.Handle([&](std::variant<int, std::exception_ptr> e) {
        EXPECT_EQ(std::get<int>(e), 5);
        handled = true;
    });
    EXPECT_FALSE(handled);
    event.Fire();
    EXPECT_TRUE(handled);
}

TEST(Callback, IntNestedReturnCalledImmediately) {
    bool handled = false;
    Event event;
    auto funcCoro = [&]() -> AsyncTask<> { co_await event.Wait(); };
    auto coroRunner = [&]() -> CallbackTask<int> {
        co_await funcCoro();
        co_return 5;
    };
    auto task = coroRunner();
    event.Fire();
    EXPECT_FALSE(handled);
    task.Handle([&](std::variant<int, std::exception_ptr> e) {
        EXPECT_EQ(std::get<int>(e), 5);
        handled = true;
    });
    EXPECT_TRUE(handled);
}

TEST(Callback, IntReturnCalledImmediately) {
    bool handled = false;
    Event event;
    auto coroRunner = [&]() -> CallbackTask<int> {
        co_await event.Wait();
        co_return 5;
    };
    auto task = coroRunner();
    event.Fire();
    EXPECT_FALSE(handled);
    task.Handle([&](std::variant<int, std::exception_ptr> e) {
        EXPECT_EQ(std::get<int>(e), 5);
        handled = true;
    });
    EXPECT_TRUE(handled);
}

TEST(Callback, IntReturnCalledAfter) {
    bool handled = false;
    Event event;
    auto coroRunner = [&]() -> CallbackTask<int> {
        co_await event.Wait();
        co_return 5;
    };
    auto task = coroRunner();
    EXPECT_FALSE(handled);
    task.Handle([&](std::variant<int, std::exception_ptr> e) {
        EXPECT_EQ(std::get<int>(e), 5);
        handled = true;
    });
    EXPECT_FALSE(handled);
    event.Fire();
    EXPECT_TRUE(handled);
}

TEST(Callback, VoidThrowException) {
    bool handled = false;
    Event event;
    auto coroRunner = [&]() -> CallbackTask<void> {
        co_await event.Wait();
        throw std::runtime_error("error");
    };
    auto task = coroRunner();
    EXPECT_FALSE(handled);
    task.Handle([&](std::exception_ptr e) {
        try {
            std::rethrow_exception(e);
        } catch (std::exception &e) {
            EXPECT_STREQ(e.what(), "error");
        }
        handled = true;
    });
    EXPECT_FALSE(handled);
    event.Fire();
    EXPECT_TRUE(handled);
}

TEST(Callback, IntThrowException) {
    bool handled = false;
    Event event;
    auto coroRunner = [&]() -> CallbackTask<int> {
        co_await event.Wait();
        throw std::runtime_error("error");
    };
    auto task = coroRunner();
    EXPECT_FALSE(handled);
    task.Handle([&](std::variant<int, std::exception_ptr> e) {
        try {
            std::rethrow_exception(std::get<std::exception_ptr>(e));
        } catch (std::exception &e) {
            EXPECT_STREQ(e.what(), "error");
        }
        handled = true;
    });
    EXPECT_FALSE(handled);
    event.Fire();
    EXPECT_TRUE(handled);
}