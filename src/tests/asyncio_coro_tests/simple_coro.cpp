#include <gtest/gtest.h>

#include <asyncio_coro/async_task.h>
#include <asyncio_coro/timer_context.h>
#include <asyncio_coro/socket_context.h>
#include <asyncio_coro/interval.h>
#include <asyncio_coro/buffer.h>
#include <asyncio_coro/socket.h>
#include <asyncio_coro/listening_socket.h>
#include <asyncio_coro/linked_socket_context.h>
#include <asyncio_coro/IpAddress.h>

namespace SimpleCoroTest {

AsyncTask<> child_task(TimerContext &timerContext) {
    for (int i = 0; i < 3; i++) {
        std::cout << "Child Task" << std::endl;
        co_await async_wait_for_ms(10, timerContext);
    }
}

coroutine_handle<> global_handle;

AsyncTask<> never_ending_task() {
    struct Awaitable {
        constexpr bool await_ready() const noexcept { return false; }
        void await_resume() const noexcept {}
        void await_suspend(coroutine_handle<> h) { global_handle = h; }
    };
    auto awaitable = Awaitable();
    co_await awaitable;

    int w;
}

AsyncTask<> parent_task(TimerContext &timerContext) {
    co_await never_ending_task();

    for (int i = 0; i < 3; i++) {
        co_await async_wait_for_ms(30, timerContext);
        std::cout << "Parent Task" << std::endl;
        child_task(timerContext);
    }
    co_return;
}

AsyncTask<> socket_task(SocketContext &context) {
    std::cout << "Socket task Start" << std::endl;

    auto socket = Socket::create(context);

    // if (false) {
    co_await socket->read_operation.wait();
    // }
    // auto server = ListeningSocket::create(context);

    // if (server) {
    //     if (server->start_listening(1235)) {
    //         server->accept();
    //         if (socket) {
    //             auto address = IPAddress::loopback(1235);

    //             std::cout << "Before Connection" << std::endl;

    //             co_await socket->connect(address);
    //             std::cout << "Connected" << std::endl;

    //             co_await write_async_c_string("hello there", socket->write_operation);

    //             std::cout << "Wrote: hello there" << std::endl;

    //             // char data[10];
    //             // try {
    //             //     co_await socket->read_operation((char *)&data, 10);
    //             // } catch (std::exception e) {
    //             //     int j = 0;
    //             // }

    //             int w = 0;
    //         }
    //     }
    // }
}

TEST(SimpleCoroTest, Start) {

    // TimerContext context;
    SocketContext socketContext;

    // auto task = parent_task(context);

    // never_ending_task();

    socket_task(socketContext);
    int w = 0;

    std::cout << "here" << std::endl;

    socketContext.poll();
    socketContext.poll();
    socketContext.poll();
    socketContext.poll();
    socketContext.poll();
    socketContext.poll();
}

struct Awaitable {
    coroutine_handle<> h;
    bool await_ready() const noexcept { return false; }
    void await_resume() const noexcept {}
    void await_suspend(coroutine_handle<> h) noexcept { this->h = h; }
};

Awaitable awaitable;

TEST(AsyncTaskTest, Void_Return_Immediate_Detached_Top) {

    auto task = []() -> AsyncTask<> { co_return; };

    task();
}

TEST(AsyncTaskTest, Value_Return_Immediate_Detached_Top) {

    auto func = []() -> AsyncTask<int> { co_return 5; };

    auto task = func();

    int top_result = 0;
    if (task.await_ready()) {
        top_result = task.await_resume();
    }

    ASSERT_EQ(top_result, 5);
}

TEST(AsyncTaskTest, Void_Block_Till_Death_Detached_Top) {
    awaitable = Awaitable();

    auto task = []() -> AsyncTask<> { co_await awaitable; };

    task();

    awaitable.h.destroy();
}

TEST(AsyncTaskTest, Return_Value_Block_Till_Death_Detached_Top) {
    awaitable = Awaitable();
    int top_result = 0;

    auto child_task = []() -> AsyncTask<int> {
        co_await awaitable;
        throw std::exception();

        co_return 5;
    };

    auto task = [&]() -> AsyncTask<> { top_result = co_await child_task(); };

    task();

    // resume to throw an exception
    awaitable.h.resume();

    ASSERT_EQ(top_result, 0);
}

TEST(AsyncTaskTest, Return_Void_Block_Till_Death_Detached_Top) {
    awaitable = Awaitable();
    int top_result = 0;

    auto child_task = [&]() -> AsyncTask<> {
        co_await awaitable;
        throw std::exception();
        top_result = 5;

        co_return;
    };

    auto task = [&]() -> AsyncTask<> { co_await child_task(); };

    task();

    // resume to throw an exception
    awaitable.h.resume();

    ASSERT_EQ(top_result, 0);
}

TEST(AsyncTaskTest, Return_Value_Block_Temporally_Detached_Top) {

    awaitable = Awaitable();

    auto child_task = []() -> AsyncTask<int> {
        co_await awaitable;
        co_return 5;
    };

    int top_result = 0;

    auto task = [&]() -> AsyncTask<> {
        auto result = co_await child_task();
        top_result = result;
    };

    task();

    awaitable.h.resume();

    ASSERT_EQ(top_result, 5);
}

TEST(AsyncTaskTest, Return_Void_Block_Temporally_Detached_Top) {

    awaitable = Awaitable();
    int top_result = 0;

    auto child_task = [&]() -> AsyncTask<void> {
        co_await awaitable;
        top_result = 5;
        co_return;
    };

    auto task = [&]() -> AsyncTask<> { co_await child_task(); };

    task();

    awaitable.h.resume();

    ASSERT_EQ(top_result, 5);
}

TEST(AsyncTaskTest, Return_Value_Immediately_Detached_Top) {

    auto child_task = []() -> AsyncTask<int> { co_return 5; };

    int top_result = 0;

    auto task = [&]() -> AsyncTask<> {
        auto result = co_await child_task();
        top_result = result;
    };

    task();

    ASSERT_EQ(top_result, 5);
}

TEST(AsyncTaskTest, Return_Void_Immediately_Detached_Top) {

    int top_result = 0;

    auto child_task = [&]() -> AsyncTask<> {
        top_result = 5;
        co_return;
    };

    auto task = [&]() -> AsyncTask<> { co_await child_task(); };

    task();

    ASSERT_EQ(top_result, 5);
}

// TEST(AsyncTaskTest, Yeild_Value_Immediately_Detached_Top) {

//     auto child_task = []() -> AsyncTask<int> {
//         for (int i = 0; i < 3; i++) {
//             std::cout << i << std::endl;
//             co_yield i + 1;
//         }
//     };

//     int top_result0 = 0;
//     int top_result1 = 0;
//     int top_result2 = 0;

//     auto task = [&]() -> AsyncTask<> {
//         auto task = child_task();
//         top_result0 = co_await task();
//         top_result1 = co_await task();
//         top_result2 = co_await task();
//     };

//     task();

//     ASSERT_EQ(top_result0, 1);
//     ASSERT_EQ(top_result1, 2);
//     ASSERT_EQ(top_result2, 3);
// }

// TEST(AsyncTaskTest, Yeild_Value_Temporally_Detached_Top) {

//     awaitable = Awaitable();

//     auto child_task = []() -> AsyncTask<int> {
//         for (int i = 0; i < 3; i++) {
//             co_await awaitable;
//             co_yield i + 1;
//         }
//     };

//     int top_result0 = 0;
//     int top_result1 = 0;
//     int top_result2 = 0;

//     auto task = [&]() -> AsyncTask<> {
//         auto task = child_task();
//         top_result0 = co_await task();
//         top_result1 = co_await task();
//         top_result2 = co_await task();
//     };

//     task();

//     awaitable.h.resume();
//     ASSERT_EQ(top_result0, 1);
//     ASSERT_EQ(top_result1, 0);
//     ASSERT_EQ(top_result2, 0);

//     awaitable.h.resume();
//     ASSERT_EQ(top_result0, 1);
//     ASSERT_EQ(top_result1, 2);
//     ASSERT_EQ(top_result2, 0);

//     awaitable.h.resume();
//     ASSERT_EQ(top_result0, 1);
//     ASSERT_EQ(top_result1, 2);
//     ASSERT_EQ(top_result2, 3);
// }

}   // namespace SimpleCoroTest

/*

AsyncTask<int> some_task(){
    promise_type promise;
    auto return_object = promise.get_return_object();

    try{



    }catch(){

    }

    co_await promise.final_suspend();

    delete promise;
}


*/