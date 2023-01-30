#pragma once
#include <unordered_map>
#include <string>
#include <algorithm>
#include "url.h"
#include <iostream>

#include "../async_task.h"

std::string_view trim_white_space(std::string_view str);

std::unordered_map<std::string_view, std::string_view> get_header_from_string(std::string_view text);

std::string_view header_string(std::string_view text);

// assumes end byte is writeable temporally
size_t sv_to_size_t(const std::string_view &str);

bool ends_with_two_new_lines(const std::string &str);

struct HttpResponse {
    std::string str = "";
    std::unordered_map<std::string_view, std::string_view> header;
    std::string body;
};

template <typename AsyncOperation> AsyncTask<HttpResponse> http_request_buffer(AsyncOperation &read_operation) {
    HttpResponse response;
    char c;

    do {
        co_await read_operation(&c, 1);
        response.str += c;
    } while (!ends_with_two_new_lines(response.str));

    response.header = get_header_from_string(header_string(response.str));
    auto content_length = sv_to_size_t(response.header["Content-Length"]);
    response.body.append(content_length, '0');

    co_await read_operation(response.body.data(), content_length);

    co_return std::move(response);
}
