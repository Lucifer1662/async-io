#include <asyncio/context.h>
#include <asyncio/time_util.h>
#include <asyncio/interval.h>
#include <asyncio/ContextSocket.h>
#include <asyncio/socket/WSAStartup.h>
#include <gtest/gtest.h>

#include "test.h"

// Demonstrate some basic assertions.
TEST(ContextTimerTest, Single_Timer_No_Sockets) {
    AsyncioGlobal startup;
    startup.start();

    Context context(std::chrono::milliseconds(500));
    int timer_called = 0;
    context.add_timer(wait_for_seconds(1), [&]() { timer_called++; });
    long long ms;
    ms = context.step();
    ASSERT_EQ(ms, 500);
    ms = context.step();
    ASSERT_LE(ms, 500);

    ASSERT_EQ(timer_called, 1);
}

TEST(ContextTimerTest, Single_Timer_Single_Sockets) {
    AsyncioGlobal startup;
    startup.start();

    Context context(std::chrono::milliseconds(500));

    ContextSocket socket(context);
    socket.connect(IPAddress::loopback(500), [](auto &socket, auto error) {});

    int timer_called = 0;
    context.add_timer(wait_for_seconds(1), [&]() { timer_called++; });
    long long ms;
    ms = context.step();
    ASSERT_LE(ms, 500);
    ms = context.step();
    ASSERT_LE(ms, 500);

    ASSERT_EQ(timer_called, 1);
}

// Interval tests -----
TEST(ContextIntervalTest, Single_Timer_No_Sockets) {
    AsyncioGlobal startup;
    startup.start();

    Context context(std::chrono::milliseconds(500));

    int timer_called = 0;

    make_interval(context, 1000, [&]() { timer_called++; });

    long long ms;
    ms = context.step();
    ASSERT_LE(ms, 500);

    ms = context.step();
    ASSERT_LE(ms, 500);

    ms = context.step();
    ASSERT_LE(ms, 500);

    ms = context.step();
    ASSERT_LE(ms, 500);

    ms = context.step();
    ASSERT_LE(ms, 500);

    ms = context.step();
    ASSERT_LE(ms, 500);

    ASSERT_EQ(timer_called, 3);
}

TEST(ContextIntervalTest, Single_Interval_Single_Sockets) {
    AsyncioGlobal startup;
    startup.start();

    Context context(std::chrono::milliseconds(500));

    ContextSocket socket(context);
    socket.connect(IPAddress::loopback(500), [](auto &socket, auto error) {});

    int timer_called = 0;

    make_interval(context, 1000, [&]() { timer_called++; });

    context.start();

    long long ms;
    ms = context.step();
    ASSERT_LE(ms, 500);

    ms = context.step();
    ASSERT_LE(ms, 500);

    ms = context.step();
    ASSERT_LE(ms, 500);

    ms = context.step();
    ASSERT_LE(ms, 500);

    ms = context.step();
    ASSERT_LE(ms, 500);

    ms = context.step();
    ASSERT_LE(ms, 500);

    ms = context.step();
    ASSERT_LE(ms, 500);

    ASSERT_EQ(timer_called, 3);
}

TEST(ContextIntervalTest, Single_Interval_Long_Running) {
    AsyncioGlobal startup;
    startup.start();

    Context context(std::chrono::milliseconds(500));

    ContextSocket socket(context);
    socket.connect(IPAddress::loopback(500), [](auto &socket, auto error) {});

    int timer_called = 0;

    make_interval(context, 1000, [&]() {
        timer_called++;
        Sleep(1800);
    });

    context.start();

    long long ms;
    ms = context.step();
    ASSERT_LE(ms, 500);

    ms = context.step();
    ASSERT_LE(ms, 500);

    ms = context.step();
    ASSERT_LE(ms, 500);

    ms = context.step();
    ASSERT_LE(ms, 500);

    ms = context.step();
    ASSERT_LE(ms, 500);

    ms = context.step();
    ASSERT_LE(ms, 500);

    ms = context.step();
    ASSERT_LE(ms, 500);

    ASSERT_EQ(timer_called, 6);
}

TEST(ContextWaitTime, No_timers_No_comp_time) { ASSERT_EQ(wait_time(500, 500, LLONG_MAX, 0), 500); }
TEST(ContextWaitTime, No_timers_Some_comp_time) { ASSERT_EQ(wait_time(500, 501, LLONG_MAX, 0), 499); }
TEST(ContextWaitTime, Some_timers_In_Interval_No_comp_time) { ASSERT_EQ(wait_time(500, 500, 499, 0), 499); }
TEST(ContextWaitTime, Some_timers_Not_In_Interval_No_comp_time) { ASSERT_EQ(wait_time(500, 500, 501, 0), 500); }
TEST(ContextWaitTime, Some_timers_In_Interval_comp_time_equal) { ASSERT_EQ(wait_time(500, 501, 499, 0), 499); }
TEST(ContextWaitTime, Some_timers_In_Interval_comp_time_less) { ASSERT_EQ(wait_time(500, 501, 498, 0), 498); }
TEST(ContextWaitTime, Some_timers_In_Interval_comp_time_more) { ASSERT_EQ(wait_time(500, 502, 499, 0), 498); }
