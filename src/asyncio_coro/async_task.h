#pragma once
#include "coroutine.h"
#include <optional>
#include <vector>

template <typename promise_type> struct final_suspend_promise_type_awaiter {
    const promise_type &promise;
    final_suspend_promise_type_awaiter(const promise_type &promise) noexcept
        : promise(promise) {}

    bool await_ready() const noexcept { return false; }
    void await_resume() const noexcept {}
    void await_suspend(coroutine_handle<promise_type> h) noexcept {
        auto precursor = h.promise().precursor;
        if (precursor) {
            precursor.resume();
        }
    }
};

template <typename Return_Value = void> struct AsyncTask {
    struct promise_type {
        coroutine_handle<> precursor;
        std::optional<Return_Value> ret_value;
        bool finished = false;
        AsyncTask get_return_object() { return {coroutine_handle<promise_type>::from_promise(*this)}; }
        suspend_never initial_suspend() { return {}; }

        auto final_suspend() noexcept {
            finished = true;
            return final_suspend_promise_type_awaiter{*this};
        }

        void return_value(Return_Value value) noexcept { ret_value = std::move(value); }
        auto yield_value(Return_Value value) noexcept {
            ret_value = std::move(value);
            return final_suspend_promise_type_awaiter{*this};
        }

        void unhandled_exception() {}
    };

    coroutine_handle<promise_type> h_;
    operator coroutine_handle<promise_type>() const { return h_; }
    operator coroutine_handle<>() const { return h_; }

    bool await_ready() const noexcept { return h_.promise().finished; }

    Return_Value await_resume() const noexcept {
        if (h_.promise().ret_value.has_value())
            return std::move(*h_.promise().ret_value);
        else
            return Return_Value{};
    }

    void await_suspend(coroutine_handle<> coroutine) const noexcept {
        // The coroutine itself is being suspended (async work can beget other async work)
        // Record the argument as the continuation point when this is resumed later. See
        // the final_suspend awaiter on the promise_type above for where this gets used
        h_.promise().precursor = coroutine;
    }

    template <typename... Args> static AsyncTask<std::vector<Return_Value>> all_vec(Args &&...tasks) {
        return std::vector<Return_Value>({co_await tasks...});
    }

    template <typename... Args> static AsyncTask<std::vector<Return_Value>> all_tup(Args &&...tasks) {
        return std::make_tuple(co_await tasks...);
    }
};

template <> struct AsyncTask<void> {
    struct promise_type {
        coroutine_handle<> precursor;
        bool finished = false;
        AsyncTask get_return_object() { return {coroutine_handle<promise_type>::from_promise(*this)}; }
        suspend_never initial_suspend() { return {}; }

        auto final_suspend() noexcept {
            finished = true;
            return final_suspend_promise_type_awaiter{*this};
        }

        void return_void() const noexcept {}
        void unhandled_exception() {}

        // AsyncTask<Return_Value> return_value() { return {coroutine_handle<promise_type>::from_promise(*this)}; }
    };

    coroutine_handle<promise_type> h_;
    operator coroutine_handle<promise_type>() const { return h_; }
    // A coroutine_handle<promise_type> converts to coroutine_handle<>
    operator coroutine_handle<>() const { return h_; }

    bool await_ready() const noexcept { return h_.promise().finished; }

    void await_resume() const noexcept {}

    void await_suspend(coroutine_handle<> coroutine) const noexcept {
        // The coroutine itself is being suspended (async work can beget other async work)
        // Record the argument as the continuation point when this is resumed later. See
        // the final_suspend awaiter on the promise_type above for where this gets used
        h_.promise().precursor = coroutine;
    }

    template <typename... Args> static AsyncTask<> all(Args &&...tasks) {
        //  std::vector<Return_Value>({co_await tasks...});
        co_return;
    }
};