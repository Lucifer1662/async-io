#pragma once

// struct CoroContext {
//     std::list<std::list<AsyncTask>> resumable_coroutines;

//     void make_resumable(coroutine_handle<> handle) { resumable_coroutines.push_back(handle); }

//     void resume() {
//         auto initial_size = resumable_coroutines.size();
//         auto it = resumable_coroutines.begin();
//         for (size_t i = 0; i < initial_size; i++) {
//             it->resume();
//         }

//         resumable_coroutines.erase(resumable_coroutines.begin(), it);
//     }

//     auto suspend() {
//         struct Awaitable {
//             CoroContext &context;
//             constexpr bool await_ready() const noexcept { return false; }
//             void await_resume() const noexcept {}
//             void await_suspend(coroutine_handle<AsyncTask::promise_type> h) { context.make_resumable(h); }
//         };

//         return Awaitable{*this};
//     }
// };