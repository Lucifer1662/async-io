#pragma once
#include <unordered_map>
#include <string>
#include <algorithm>
#include "url.h"
#include <iostream>

#include "../async_task.h"

namespace Http {

std::string_view trim_white_space(std::string_view str);

std::unordered_map<std::string_view, std::string_view> get_header_from_string(std::string_view text);

std::string_view header_string(std::string_view text);

// assumes end byte is writeable temporally
size_t sv_to_size_t(const std::string_view &str);

bool ends_with_two_new_lines(const std::string &str);
bool ends_with_single_new_lines(const std::string &str);

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

    std::cout << response.str << std::endl;

    response.header = get_header_from_string(header_string(response.str));

    auto it = response.header.find("Content-Length");

    if (it != response.header.end()) {
        auto content_length = sv_to_size_t(it->second);
        response.body.append(content_length, '0');
        co_await read_operation(response.body.data(), content_length);
    } else {

        it = response.header.find("Transfer-Encoding");
        if (it != response.header.end()) {
            if (it->second.find_first_of("chunked") != std::string_view::npos) {
                std::string chunk_size_str;
                char end_trail[2];
                unsigned long chunk_size;
                // read chunks
                do {
                    chunk_size_str.clear();
                    // read chunk size
                    do {
                        co_await read_operation(&c, 1);
                        chunk_size_str += c;
                    } while (!ends_with_single_new_lines(chunk_size_str));

                    std::cout << "Chunk String: " << chunk_size_str << std::endl;

                    chunk_size = std::stoul(chunk_size_str.c_str(), nullptr, 16);

                    std::cout << chunk_size << std::endl;

                    response.body.append(chunk_size, '0');
                    co_await read_operation(response.body.data(), chunk_size);

                    co_await read_operation((char *)&end_trail, 2);

                } while (chunk_size > 0);
            }
        }
    }

    co_return std::move(response);
}

}   // namespace Http