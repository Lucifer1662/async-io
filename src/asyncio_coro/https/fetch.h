#pragma once
#include "../http/fetch.h"
#include "../ssl/ssl_context.h"
#include "../ssl/ssl_socket.h"

namespace Https {
template <typename Func>
AsyncTask<Http::HttpResponse> fetch(const Http::FetchParams &params, SocketContext &socketContext,
                                    SSLContext &sslContext, Http::Body<Func> &body) {
    auto socket_opt = SSL_Socket::create(socketContext, sslContext);
    if (socket_opt) {
        co_return co_await fetch_with_socket(params, *socket_opt, body);
    }
}

AsyncTask<Http::HttpResponse> fetch(const Http::FetchParams &params, SocketContext &socketContext,
                                    SSLContext &sslContext) {
    auto socket_opt = SSL_Socket::create(socketContext, sslContext);
    if (socket_opt) {
        auto body = Http::Body([](auto &s) -> AsyncTask<> { co_return; });
        co_return co_await fetch_with_socket(params, *socket_opt, body);
    }
}

}   // namespace Https