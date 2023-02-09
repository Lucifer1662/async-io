#include "asyncio_coro/linked_socket_context.h"
#include "asyncio_coro/timer_context.h"
#include "asyncio_coro/socket_context.h"
#include "asyncio_coro/interval.h"

void link_socket_to_timer_context(TimerContext &timerContext, SocketContext &socketContext,
                                  std::chrono::milliseconds interval) {
    make_interval_at(timerContext, interval.count(), current_time_ms(), [&]() { socketContext.poll(); });
}
