#pragma once
#include "coroutine.h"
#include <optional>
#include <vector>
#include <tuple>
#include <stdio.h>
#include <exception>
#include <memory>
#include <variant>

struct final_suspend_promise_type_awaiter {
    const coroutine_handle<> &precursor;
    final_suspend_promise_type_awaiter(coroutine_handle<> &precursor) noexcept
        : precursor(precursor) {}

    constexpr bool await_ready() const noexcept { return true; }
    void await_resume() const noexcept {
        if (precursor) {
            precursor.resume();
        }
    }
    void await_suspend(coroutine_handle<> h) const noexcept {}
};

struct partial_promise_type {

    // partial_promise_type() { printf("Created: %d\n", this); }

    // ~partial_promise_type() { printf("Destroyed: %d\n", this); }

    coroutine_handle<> precursor;

    suspend_never initial_suspend() { return {}; }

    auto final_suspend() noexcept { return final_suspend_promise_type_awaiter{precursor}; }
};

template <typename Return_Value = void> struct AsyncTask {

    struct promise_type : public partial_promise_type {
        std::optional<Return_Value> ret_value;
        std::shared_ptr<std::optional<std::variant<Return_Value, std::exception_ptr>>> ret_value1;

        AsyncTask get_return_object() {
            ret_value1 = std::make_shared<std::optional<std::variant<Return_Value, std::exception_ptr>>>();
            return {coroutine_handle<promise_type>::from_promise(*this), ret_value1};
        }
        void return_value(Return_Value value) noexcept {
            // ret_value.emplace(std::move(value));
            ret_value1->emplace(std::move(value));
        }

        void unhandled_exception() { ret_value1->emplace(std::current_exception()); }

        ~promise_type() { int w = 0; }
    };

    coroutine_handle<promise_type> h_;
    std::shared_ptr<std::optional<std::variant<Return_Value, std::exception_ptr>>> ret_value1;

    operator coroutine_handle<promise_type>() const { return h_; }
    operator coroutine_handle<>() const { return h_; }

    bool await_ready() const noexcept { return ret_value1->has_value(); }

    Return_Value await_resume() const {
        // auto ret_value = std::move(h_.promise().ret_value);
        // if (h_.promise().precursor) {
        // h_.destroy();
        // }

        if (std::holds_alternative<Return_Value>(**ret_value1)) {
            return std::move(std::get<Return_Value>(**ret_value1));
        } else {
            std::rethrow_exception(std::get<std::exception_ptr>(**ret_value1));
        }
    }

    void await_suspend(coroutine_handle<> coroutine) const noexcept { h_.promise().precursor = coroutine; }

    template <typename... Args> static AsyncTask<std::vector<Return_Value>> all_vec(Args &&...tasks) {
        return std::vector<Return_Value>({co_await tasks...});
    }

    template <typename... Args> static AsyncTask<std::vector<Return_Value>> all_tup(Args &&...tasks) {
        return std::make_tuple(co_await tasks...);
    }

    AsyncTask<Return_Value> &operator()() {
        h_.resume();
        return *this;
    }
};

struct Void {};

template <> struct AsyncTask<void> {
    struct promise_type : partial_promise_type {
        std::shared_ptr<std::optional<std::variant<Void, std::exception_ptr>>> ret_value1;

        AsyncTask get_return_object() {
            ret_value1 = std::make_shared<std::optional<std::variant<Void, std::exception_ptr>>>();
            return {coroutine_handle<promise_type>::from_promise(*this), ret_value1};
        }

        void return_void() const noexcept { *ret_value1 = Void(); }

        void unhandled_exception() { *ret_value1 = std::current_exception(); }
    };

    coroutine_handle<promise_type> h_;
    std::shared_ptr<std::optional<std::variant<Void, std::exception_ptr>>> ret_value1;

    operator coroutine_handle<promise_type>() const { return h_; }
    // A coroutine_handle<promise_type> converts to coroutine_handle<>
    operator coroutine_handle<>() const { return h_; }

    bool await_ready() const noexcept { return ret_value1->has_value(); }

    void await_resume() const {

        if (std::holds_alternative<std::exception_ptr>(**ret_value1)) {
            std::rethrow_exception(std::get<std::exception_ptr>(**ret_value1));
        }
    }

    void await_suspend(coroutine_handle<> coroutine) const noexcept { h_.promise().precursor = coroutine; }

    template <typename... Args> static AsyncTask<> all(Args &&...tasks) {
        //  std::vector<Return_Value>({co_await tasks...});
        co_return;
    }
};