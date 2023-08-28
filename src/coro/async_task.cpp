#include "async_task.h"

AsyncTask<void> Promise<void>::get_return_object() { return {coroutine_handle<Promise<void>>::from_promise(*this)}; }
