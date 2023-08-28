#include "callback_task.h"

template <> struct CallbackTask<void>;

CallbackTask<void> CallbackPromise<void>::get_return_object() {
    return {coroutine_handle<CallbackPromise<void>>::from_promise(*this)};
}
