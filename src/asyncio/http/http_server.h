#pragma once
#include <asyncio/ContextSocket.h>
#include <asyncio/http/http_request_buffer.h>
#include <list>

template <typename Func> void WhenEmpty(ContextSocket &socket, Func &&func) {
    socket.write(std::make_unique<Buffer>(), [&, func](auto buffer, bool error) {
        if (socket.wants_to_write() == 1) {
            func(true);
        } else {
            if (func(false)) {
                socket.write(std::make_unique<Buffer>(),
                             [=, func, &socket](auto buffer, bool error) { WhenEmpty(socket, func); });
            }
        }
    });
}

struct HttpServer {
    ContextListeningSocket socket;
    std::list<ContextSocket> sockets;

    template <typename Func>
    HttpServer(Context &context, Func &&on_request)
        : socket(context, [&](ContextSocket &&sock) {
              // hold onto socket during async event
              sockets.emplace_front(sock);
              auto socket_it = sockets.begin();

              socket_it->read(std::make_unique<HttpBufferReceiver>(),
                              [on_request = std::forward<Func>(on_request), socket_it,
                               this](std::unique_ptr<HttpBufferReceiver> buffer, bool error) {
                                  // new http request
                                  if (!error) {
                                      on_request(std::move(buffer), *socket_it);

                                      // after async event, we can remove it
                                      WhenEmpty(*socket_it, [socket_it, this](bool is_empty) {
                                          if (is_empty) {
                                              this->sockets.erase(socket_it);
                                              return false;
                                          }
                                          return true;
                                      });
                                  }
                              });
          }) {}

    bool start(int port = 80) { return socket.start_listening(port); }
};