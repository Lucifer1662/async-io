#include "linked_socket_context.h"
#include "timer_context.h"
#include "socket_context.h"
#include "interval.h"

void link_socket_to_timer_context(TimerContext &timerContext, SocketContext &socketContext,
                                  std::chrono::milliseconds interval) {
    make_interval_at(timerContext, interval.count(), current_time_ms(), [&]() { socketContext.step(); });
}
