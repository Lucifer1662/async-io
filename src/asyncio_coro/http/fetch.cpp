#include "fetch.h"

namespace Http {

AsyncTask<HttpResponse> fetch(const FetchParams &params, SocketContext &socketContext) {
    auto socket_opt = Socket::create(socketContext);
    if (socket_opt) {
        auto body = Body([](auto &s) -> AsyncTask<> { co_return; });
        co_return co_await fetch_with_socket(params, *socket_opt, body);
    }
}

}   // namespace Http
