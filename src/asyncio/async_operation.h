#pragma once
#include <queue>
#include <functional>
#include "buffer/buffer.h"

using Buffer_Ptr = std::unique_ptr<Buffer>;

template <typename T, typename S>
std::unique_ptr<T> dynamic_pointer_cast(std::unique_ptr<S> &&p) noexcept
{
    auto converted = std::unique_ptr<T>{dynamic_cast<T *>(p.get())};
    if (converted)
    {
        p.release(); // no longer owns the pointer
    }
    return converted;
}

template <typename BaseOp>
class AsyncOperation
{
    struct Request
    {
        std::function<void(Buffer_Ptr, bool)> callback;
        Buffer_Ptr buffer;
        template <typename Func>
        Request(Func &&callback, Buffer_Ptr buffer)
            : callback(std::forward<Func>(callback)), buffer(std::move(buffer)) {}

        Request(const Request &) = default;
        Request(Request &&) = default;
        Request &operator=(const Request &) = default;
    };
    std::queue<Request> operation_requests;
    bool can_perform;

public:
    BaseOp base_operation;

    AsyncOperation(bool can_perform, BaseOp base_operation)
        : can_perform(can_perform), base_operation(std::move(base_operation))
    {
    }

    bool wants_more()
    {
        return !operation_requests.empty();
    }

    // call back is void(Buffer_Ptr, bool) type
    template <typename Buffer_T, typename Func>
    void request(std::unique_ptr<Buffer_T> b, Func &&callback)
    {
        // if not already subscribe in context to type of request
        // alternatively if a no op occurs unsubscribe

        // if potentially there is room, we may as well try and use it straight away
        if (can_perform)
        {
            if (operation(*b))
            {
                callback(std::move(b), false);
                return;
            }
            else
            {
                can_perform = false;
            }
        }

        operation_requests.emplace(
            [callback = std::forward<Func>(callback)](Buffer_Ptr buffer, bool error)
            {
                callback(dynamic_pointer_cast<Buffer_T>(std::move(buffer)), error);
            },
            std::move(b));
    }

    bool check_requests()
    {
        while (!operation_requests.empty())
        {
            auto &front = operation_requests.front();
            auto overwrote = operation(*front.buffer) if (overwrote == 0)
            {
                front.callback(std::move(front.buffer), false);
                operation_requests.pop();
            }
            else if (overwrote == -1)
            {
                break;
            }
        }

        // if there are no operation requests remaining, or there was none to begin with,
        // then there potentially may be some more operations that could be requested
        // however if there are more operations left, then we can assume we have consumed as much as possible
        can_perform = operation_requests.empty();

        // return that we want to unsubscribe
        return operation_requests.empty();
    }

private:
    // int operation(Buffer &b)
    // {
    //     for (;;)
    //     {
    //         auto [data, size] = b.contiguous();
    //         if (size == 0)
    //             return 0;
    //         int num_written = base_operation(data, size);
    //         int num_used = b.advance(num_written);
    //         if (num_written != size)
    //         {
    //             return -1;
    //         }
    //         else
    //         {
    //             return num_used;
    //         }
    //     }
    // }

    template<typename Func>
    int operation(Buffer &b, Func&& func)
    {
        for (;;)
        {
            auto [data, size] = b.contiguous();
            if (size == 0)
                return 0;
            int num_written = func(data, size);
            int num_used = b.advance(num_written);
            if (num_written != size)
            {
                return -1;
            }
            else
            {
                return num_used;
            }
        }
    }
};
