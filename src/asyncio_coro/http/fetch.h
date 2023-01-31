#pragma once
#include "url.h"
#include "../async_task.h"
#include "../async_operation.h"
#include "../socket.h"
#include "http_request.h"
#include "../IpAddress.h"

#include <sstream>
#include <utility>
#include <functional>

struct Body {
    std::function<AsyncTask<>(Socket &socket)> body = [](auto &) -> AsyncTask<> { co_return; };
    size_t length = 0;
};

struct FetchParams {
    FetchParams() = default;
    FetchParams(FetchParams &&) = default;
    FetchParams &operator=(const FetchParams &) = default;

    URL url;
    Method method = Method::GET;
    HttpVersion version = HttpVersion(1, 1);
    std::unordered_map<std::string, std::string> headers;
    Body body;
};

class SocketContext;

AsyncTask<HttpResponse> fetch(const FetchParams &params, SocketContext &socketContext) {
    auto socket_opt = Socket::create(socketContext);

    if (socket_opt) {
        auto &socket = *socket_opt;

        auto addresses = IPAddress_from_URL(params.url.host.c_str(), "http");
        if (!addresses.empty()) {
            co_await socket.connect(addresses.front());

            RequestLine line(params.method, params.url.path, params.url.protocol, params.version);

            //    //    Host: localhost:1235\r\nUser-Agent: curl/7.83.1\r\nAccept:
            //    */*\r\nContent-Length:
            //    //    11\r\nContent-Type: application/x-www-form-urlencoded
            std::stringstream ss;
            ss << "Host: " << params.url.to_host_path() << "\r\n";
            ss << "User-Agent: "
               << "\r\n";
            ss << "Accept: */*\r\n";
            ss << "Content-Length: " << std::to_string(params.body.length) << "\r\n";
            ss << "\r\n";

            auto s = ss.str();
            auto rl = line.to_string();
            std::cout << rl;
            std::cout << s;
            co_await socket.write_operation(rl.data(), rl.size());
            co_await socket.write_operation(s.data(), s.size());
            co_await params.body.body(socket);
            auto http_response = co_await http_request_buffer(socket.read_operation);
            co_return http_response;
        }
    }
}