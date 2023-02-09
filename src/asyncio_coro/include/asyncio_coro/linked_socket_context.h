#pragma once
#include <chrono>
class TimerContext;
class SocketContext;

void link_socket_to_timer_context(TimerContext &timerContext, SocketContext &socketContext,
                                  std::chrono::milliseconds interval);
