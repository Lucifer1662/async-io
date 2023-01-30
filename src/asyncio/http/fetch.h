#pragma once
#include "../http/http_request_buffer.h"
#include "../buffer/ContainerBufferSender.h"
#include "../context.h"
#include "../ContextSocket.h"
#include "url.h"
#include <list>

#include <sstream>
#include <utility>

struct Body {
    Buffer_Ptr body = std::make_unique<Buffer>();
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

struct Fetcher {
    Context &context;
    std::list<ContextSocket> active_connections;
    Fetcher(Context &context) : context(context) {}

    template <typename Func> bool fetch(FetchParams params, Func &&callback) {
        active_connections.emplace_back(context);
        auto socket = active_connections.back();

        auto ips = IPAddress_from_URL(params.url.host.c_str(), "http");
        if (ips.empty())
            return false;

        auto p = std::make_shared<FetchParams>(std::move(params));

        // socket.connect(ips.front(), [params](auto &socket, bool error) {});

        socket.connect(ips.front(), [callback = std::forward<Func>(callback), params = p](auto &socket, bool error) {
            // connected
            // send http request

            RequestLine line(params->method, params->url.path, params->url.protocol, params->version);

            //    //    Host: localhost:1235\r\nUser-Agent: curl/7.83.1\r\nAccept:
            //    */*\r\nContent-Length:
            //    //    11\r\nContent-Type: application/x-www-form-urlencoded
            std::stringstream ss;
            ss << "Host: " << params->url.to_host_path() << "\r\n";
            ss << "User-Agent: "
               << "\r\n";
            ss << "Accept: */*\r\n";
            ss << "Content-Length: " << std::to_string(params->body.length) << "\r\n";
            ss << "\r\n";
            auto header_buffer = std::make_unique<ContainerBufferSender<std::string>>(ss.str());

            socket.write(
                std::make_unique<HttpBufferSender>(line, std::move(header_buffer), std::move(params->body.body)),
                [&, callback](std::unique_ptr<HttpBufferSender> buffer, bool error) {
                    if (!error) {
                        socket.read(std::make_unique<HttpBufferReceiver>(), callback);
                    }
                });
        });
    }
};