#pragma once
#include <assert.h>

class Event {
    coroutine_handle<> mCoroutine = {};
    std::exception_ptr mException = nullptr;
    size_t mPreFiredCount = 0;

  public:
    Event(size_t startCount)
        : mPreFiredCount(startCount) {}
    Event() = default;
    Event(const Event &) = delete;
    Event(Event &&) = delete;

    auto Wait() noexcept {
        struct Awaitable {
            Event &mEvent;

            // ready if not suspending on waiting, or there is an exception
            bool await_ready() const noexcept { return mEvent.mPreFiredCount > 0 || mEvent.mException; }

            // capture coroutine handle to resume later
            void await_suspend(coroutine_handle<> coroutine) { mEvent.mCoroutine = coroutine; }

            // when resuming
            void await_resume() {
                // check if an exception and throw to caller of co_await Wait()
                if (mEvent.mException) {
                    std::rethrow_exception(mEvent.mException);
                } else {
                    mEvent.mPreFiredCount--;
                }
            }

            // if the caller coroutine (co_await Wait()) dies
            // unlink the coroutine handle from the event
            // so that Fire() will do nothing
            ~Awaitable() { mEvent.mCoroutine = nullptr; }
        };

        // only 1 subscriber at a time
        assert(!mCoroutine);

        return Awaitable{*this};
    }

    bool FireException(const std::exception &exception) {
        // grab exception_ptr
        try {
            throw exception;
        } catch (...) {
            mException = std::current_exception();
        }

        return Fire();
    }

    bool Fire() {
        // if someone is waiting (co_await Wait())
        if (mCoroutine) {
            // resume co_await Wait() caller, however swap out mCoroutine to null
            // as we resume immediately which could cause another Fire()/ co_await pair
            auto coroutine = mCoroutine;
            mCoroutine = {};
            coroutine.resume();
            return true;
        } else {
            // next co_await Wait(), does not need to suspend
            mPreFiredCount++;
            return false;
        }
    }

    ~Event() { FireException(std::runtime_error("destruct")); }
};
