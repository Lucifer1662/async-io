#pragma once
#include "async_task.h"

template <typename ReturnValue> struct CallbackTask;
template <> struct CallbackTask<void>;

template <typename CallbackPromise> struct CallbackPromiseFinalAwaiter {
    CallbackPromise &mPromise;

    // return false to manually destroy memory in Task
    constexpr bool await_ready() const noexcept { return false; }
    void await_resume() const noexcept {}
    void await_suspend(coroutine_handle<> h) const noexcept {
        if (mPromise.mOnDoneHandler) {
            mPromise.mOnDoneHandler(mPromise.Result());
        }
    }
};

template <typename ReturnValue = void> struct CallbackPromise : public Promise<ReturnValue> {
    std::function<void(std::variant<ReturnValue, std::exception_ptr> &&)> mOnDoneHandler;
    using Promise<ReturnValue>::mResult;

    CallbackTask<ReturnValue> get_return_object() {
        return {coroutine_handle<CallbackPromise<ReturnValue>>::from_promise(*this)};
    }

    std::variant<ReturnValue, std::exception_ptr> Result() {
        return std::holds_alternative<std::exception_ptr>(mResult)
            ? std::variant<ReturnValue, std::exception_ptr>(std::get<std::exception_ptr>(mResult))
            : std::variant<ReturnValue, std::exception_ptr>(std::move(std::get<ReturnValue>(mResult)));
    }

    auto final_suspend() noexcept { return CallbackPromiseFinalAwaiter<CallbackPromise<ReturnValue>>{*this}; }
};

template <> struct CallbackPromise<void> : public Promise<void> {
    std::function<void(std::exception_ptr)> mOnDoneHandler;
    using Promise<void>::mResult;

    CallbackTask<void> get_return_object();

    std::exception_ptr Result() {
        return std::holds_alternative<std::exception_ptr>(mResult) ? std::get<std::exception_ptr>(mResult)
                                                                   : std::exception_ptr();
    }

    auto final_suspend() noexcept { return CallbackPromiseFinalAwaiter<CallbackPromise<void>>{*this}; }
};

template <typename ReturnValue> struct BaseCallbackTask : BaseAsyncTask<CallbackPromise<ReturnValue>> {
    using BaseAsyncTask<CallbackPromise<ReturnValue>>::mCoroutine;

    // Func = void(std::execption_ptr)
    template <typename Func> void Handle(Func &&handler) {
        if (mCoroutine.done()) {
            handler(mCoroutine.promise().Result());
        } else {
            // Func = void(std::execption_ptr)
            mCoroutine.promise().mOnDoneHandler = std::forward<Func>(handler);
        }
    }
};

template <typename ReturnValue = void> struct CallbackTask : public BaseCallbackTask<ReturnValue> {};
template <> struct CallbackTask<void> : public BaseCallbackTask<void> {};