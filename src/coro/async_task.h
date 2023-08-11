#pragma once
#include "coroutine.h"
#include <optional>
#include <vector>
#include <tuple>
#include <stdio.h>
#include <exception>
#include <memory>
#include <variant>

struct Empty {};
struct Void {};

template <typename ReturnValue> struct BasePromise {
    std::variant<ReturnValue, Empty, std::exception_ptr> mResult = Empty();
    bool mStarted = false;

    BasePromise() = default;
    BasePromise(const BasePromise &) = delete;
    BasePromise(const BasePromise &&) = delete;

    coroutine_handle<> precursor;
    suspend_always initial_suspend() { return {}; }
    auto final_suspend() noexcept {
        struct Awaiter {
            const coroutine_handle<> &precursor;

            // return false will always suspend the coroutine when finishing
            // this prevents the return value from being destroyed
            // then in ~AsyncTask we destroy the coroutine manually
            constexpr bool await_ready() const noexcept { return false; }
            void await_resume() const noexcept {}
            void await_suspend(coroutine_handle<> h) const noexcept {
                if (precursor) {
                    precursor.resume();
                }
            }
        };
        return Awaiter{precursor};
    }

    void unhandled_exception() { mResult = std::current_exception(); }

    void result() const {
        if (std::holds_alternative<std::exception_ptr>(mResult)) {
            std::rethrow_exception(std::get<std::exception_ptr>(mResult));
        }
    }
};

template <typename ReturnValue> struct AsyncTask;

template <typename ReturnValue = void> struct Promise : public BasePromise<ReturnValue> {

    using BasePromise<ReturnValue>::mResult;
    using ReturnValueRef = ReturnValue &&;

    AsyncTask<ReturnValue> get_return_object() { return {coroutine_handle<Promise<ReturnValue>>::from_promise(*this)}; }

    void return_value(ReturnValue &&value) noexcept { mResult = std::forward<ReturnValue>(value); }
    void return_value(const ReturnValue &value) noexcept { mResult = std::forward<ReturnValue>(value); }

    ReturnValue &&result() {
        BasePromise<ReturnValue>::result();

        return std::move(std::get<ReturnValue>(mResult));
    }
};

template <> struct Promise<void> : public BasePromise<Void> {
    using ReturnValueRef = void;

    AsyncTask<void> get_return_object();

    void return_void() noexcept { mResult = Void(); }
};

template <typename PromiseType> struct BaseAwaitable {
    coroutine_handle<PromiseType> mCoroutine;

    bool await_suspend(coroutine_handle<> h) const noexcept {
        mCoroutine.resume();
        // since we always suspend coroutines at their end,
        // it is safe to access their memory
        // the memory will be destroy on ~AsyncTask

        if (!mCoroutine.done()) {
            mCoroutine.promise().precursor = h;
            return true;
        }
        return false;
    }
    bool await_ready() const noexcept { return !mCoroutine || mCoroutine.done(); }
    PromiseType::ReturnValueRef await_resume() { return std::move(mCoroutine.promise().result()); }
};

template <typename PromiseType> struct BaseAsyncTask {
    coroutine_handle<PromiseType> mCoroutine;

    BaseAsyncTask(coroutine_handle<PromiseType> coroutine)
        : mCoroutine(coroutine){};
    BaseAsyncTask(const BaseAsyncTask &) = delete;
    BaseAsyncTask(BaseAsyncTask &&newTask) {
        if (&newTask != this) {
            newTask.mCoroutine = mCoroutine;
            mCoroutine = nullptr;
        }
    };

    using promise_type = PromiseType;
    operator coroutine_handle<PromiseType>() const { return mCoroutine; }
    operator coroutine_handle<>() const { return mCoroutine; }

    void RunDetached() const { mCoroutine.resume(); }

    auto operator co_await() const &noexcept {
        struct awaitable : public BaseAwaitable<PromiseType> {};
        return awaitable{mCoroutine};
    }

    auto WhenReady() const noexcept {
        struct awaitable : public BaseAwaitable<PromiseType> {
            void await_resume() const noexcept {}
        };
        return awaitable{mCoroutine};
    }

    ~BaseAsyncTask() {
        if (mCoroutine) {
            mCoroutine.destroy();
        }
    }
};

template <typename ReturnValue = void> struct [[nodiscard]] AsyncTask : public BaseAsyncTask<Promise<ReturnValue>> {};

template <> struct AsyncTask<void> : public BaseAsyncTask<Promise<void>> {};
