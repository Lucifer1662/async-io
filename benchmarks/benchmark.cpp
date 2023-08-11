#include <benchmark/benchmark.h>
#include <iostream>
#include <asyncio_coro/async_task.h>

struct Context {
    coroutine_handle<AsyncTask<>::promise_type> handle;

    auto await() {
        struct Awaitable {
            coroutine_handle<AsyncTask<>::promise_type> &hanlder;

            constexpr bool await_ready() const noexcept { return true; }
            void await_resume() const noexcept {}
            void await_suspend(coroutine_handle<AsyncTask<>::promise_type> h) { hanlder = h; }
        };

        return Awaitable{handle};
    }

    void resume() { handle.resume(); }
};

AsyncTask<> task1(Context &context) {
    for (int i = 0; i < 2; i++) {
        co_await context.await();
    }
}

static void SimpleResumeVoidTest(benchmark::State &state) {
    Context context;
    for (auto _ : state) {
        task1(context);
        context.resume();
        context.resume();
    }
}

BENCHMARK(SimpleResumeVoidTest);

BENCHMARK_MAIN();