#include "context.h"
#include <type_traits>

template <typename Func>
void make_interval(Context &context, long long ms, Func &&func)
{
    make_interval_at(context, ms, current_time_ms(), std::forward<Func>(func));
}


template <typename Func>
auto __make_callback(Context &context, long long ms, long long start_time, std::shared_ptr<Func> callback)
{
    return [callback, &context, ms, start_time]()
    {
        auto should_continue = (*callback)();
        if (should_continue)
            __make_interval_at(context, ms, wait_for_milliseconds(ms), callback);
    };
}

template <typename Func>
void __make_interval_at(Context &context, long long ms, long long start_time, std::shared_ptr<Func> callback)
{
    context.add_timer(start_time, __make_callback(context, ms, start_time, callback));
}

template <typename Func>
auto make_interval_at(Context &context, long long ms, long long start_time, Func &&func)
{
    // need to use shared ptr as Context uses std::function
    // which requires the object to be copyable
    // which cannot be true with a unique ptr
    auto callback = std::make_shared<Func>(std::forward<Func>(func));
    return context.add_timer(start_time, __make_callback(context, ms, start_time, callback));
}
