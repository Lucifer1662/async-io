#pragma once

#include <coroutine>
#include <optional>

using suspend_never = std::suspend_never;
using suspend_always = std::suspend_always;
template <typename T = void> using coroutine_handle = std::coroutine_handle<T>;

struct noop_coroutine_promise {};
using noop_coroutine_handle = coroutine_handle<noop_coroutine_promise>;
