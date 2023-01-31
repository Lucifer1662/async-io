#pragma once
#include "coroutine.h"
#include <optional>
#include <vector>
#include <tuple>
#include <stdio.h>
#include <exception>

struct final_suspend_promise_type_awaiter {
    const coroutine_handle<> &precursor;
    final_suspend_promise_type_awaiter(coroutine_handle<> &precursor) noexcept
        : precursor(precursor) {}

    bool await_ready() const noexcept { return true; }
    void await_resume() const noexcept {
        if (precursor) {
            return precursor.resume();
        }
    }
    void await_suspend(coroutine_handle<> h) const noexcept {}
};

struct partial_promise_type {
    std::exception_ptr exception;

    coroutine_handle<> precursor;
    suspend_never initial_suspend() { return {}; }
    auto final_suspend() noexcept { return final_suspend_promise_type_awaiter{precursor}; }
    void unhandled_exception() { this->exception = std::current_exception(); }
};

template <typename Return_Value = void> struct AsyncTask {
    struct promise_type : public partial_promise_type {
        std::optional<Return_Value> ret_value;
        AsyncTask get_return_object() { return {coroutine_handle<promise_type>::from_promise(*this)}; }
        void return_value(Return_Value value) noexcept { ret_value = std::move(value); }
        auto yield_value(Return_Value value) noexcept {
            ret_value = std::move(value);
            return final_suspend_promise_type_awaiter{precursor};
        }
    };

    coroutine_handle<promise_type> h_;
    operator coroutine_handle<promise_type>() const { return h_; }
    operator coroutine_handle<>() const { return h_; }

    bool await_ready() const noexcept { return h_.done(); }

    Return_Value await_resume() const {
        if (h_.promise().ret_value.has_value())
            return std::move(*h_.promise().ret_value);
        else if (h_.promise().exception) {
            throw h_.promise().exception;
        } else {
            throw std::exception();
        }
    }

    void await_suspend(coroutine_handle<> coroutine) const noexcept { h_.promise().precursor = coroutine; }

    template <typename... Args> static AsyncTask<std::vector<Return_Value>> all_vec(Args &&...tasks) {
        return std::vector<Return_Value>({co_await tasks...});
    }

    template <typename... Args> static AsyncTask<std::vector<Return_Value>> all_tup(Args &&...tasks) {
        return std::make_tuple(co_await tasks...);
    }
};

template <> struct AsyncTask<void> {
    struct promise_type : partial_promise_type {
        AsyncTask get_return_object() { return {coroutine_handle<promise_type>::from_promise(*this)}; }

        void return_void() const noexcept {}
    };

    coroutine_handle<promise_type> h_;
    operator coroutine_handle<promise_type>() const { return h_; }
    // A coroutine_handle<promise_type> converts to coroutine_handle<>
    operator coroutine_handle<>() const { return h_; }

    bool await_ready() const noexcept { return h_.done(); }

    void await_resume() const {
        if (h_.promise().exception) {
            throw h_.promise().exception;
        } else {
            throw std::exception();
        }
    }

    void await_suspend(coroutine_handle<> coroutine) const noexcept { h_.promise().precursor = coroutine; }

    template <typename... Args> static AsyncTask<> all(Args &&...tasks) {
        //  std::vector<Return_Value>({co_await tasks...});
        co_return;
    }
};