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
#include <exception>
class SocketContext;

namespace Http {

template <typename Func> struct Body {
    Func body;
    Body(Func &&body, size_t length = 0)
        : body(std::forward<Func>(body))
        , length(length) {}
    Body() = default;
    Body(const Body &) = default;
    Body(Body &&) = default;
    Body &operator=(const Body &) = default;

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
};

struct StringException : public std::exception {
    std::string message;

    StringException(std::string message)
        : message(std::move(message)) {}

    const char *what() const noexcept override { return message.c_str(); };
};

template <typename Socket, typename Body>
AsyncTask<HttpResponse> fetch_with_socket(const FetchParams &params, Socket &socket, Body &body) {

    auto addresses = IPAddress_from_URL(params.url.host.c_str(), params.url.protocol.to_string().c_str());
    if (!addresses.empty()) {
        std::cout << (int)addresses.front().address[0] << "." << (int)addresses.front().address[1] << "."
                  << (int)addresses.front().address[2] << "." << (int)addresses.front().address[3] << "." << std::endl;
        co_await socket.connect(addresses.front());

        RequestLine line(params.method, params.url.path, params.url.protocol, params.version);

        //    //    Host: localhost:1235\r\nUser-Agent: curl/7.83.1\r\nAccept:
        //    */*\r\nContent-Length:
        //    //    11\r\nContent-Type: application/x-www-form-urlencoded
        std::stringstream ss;
        ss << "Host: " << params.url.to_host_path() << "\r\n";
        ss << "user-agent: curl/7.74.0"
           << "\r\n";
        ss << "accept: */*\r\n";
        // ss << "Content-Length: " << std::to_string(body.length) << "\r\n";
        ss << "\r\n";

        auto s = ss.str();
        auto rl = line.to_string();
        std::cout << rl;
        std::cout << s;
        co_await socket.write_operation(rl.data(), rl.size());
        co_await socket.write_operation(s.data(), s.size());
        co_await body.body(socket);
        auto http_response = co_await http_request_buffer(socket.read_operation);
        co_return http_response;
    } else {
        throw StringException("No url found for " + params.url.host);
    }
}

template <typename Func>
AsyncTask<HttpResponse> fetch(const FetchParams &params, SocketContext &socketContext, Body<Func> &body) {
    auto socket_opt = Socket::create(socketContext);
    if (socket_opt) {
        co_return co_await fetch_with_socket(params, *socket_opt, body);
    }
}

AsyncTask<HttpResponse> fetch(const FetchParams &params, SocketContext &socketContext);

}   // namespace Http