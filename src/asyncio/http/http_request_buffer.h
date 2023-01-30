#pragma once
#include "../buffer/buffer.h"
#include <unordered_map>
#include <string>
#include <algorithm>
#include "url.h"
#include "../buffer/CompositeBuffer.h"
#include "../buffer/ContainerBufferSender.h"

std::string_view trim_white_space(std::string_view str);

std::unordered_map<std::string_view, std::string_view> get_header_from_string(std::string_view text);

std::string_view header_string(std::string_view text);

// assumes end byte is writeable temporally
size_t sv_to_size_t(const std::string_view &str);

struct HttpBufferReceiver : public Buffer {
    std::string str = "";
    std::string body_str;
    size_t position = 0;
    int state = 0;
    char c;
    std::unordered_map<std::string_view, std::string_view> header;
    size_t content_length;

    std::pair<char *, int> contiguous() override {
        if (state == 0) {
            return {&c, 1};
        } else {
            size_t a = content_length - position;
            size_t b = (size_t)INT_MAX;

            int amount = std::min(a, b);
            return {body_str.data() + position, amount};
        }
    }

    bool ends_with_two_new_lines() {
        if (str.size() < 4)
            return 0;
        auto it = str.rbegin();
        if ('\n' != *it)
            return false;
        it = std::next(it);
        if ('\r' != *it)
            return false;
        it = std::next(it);
        if ('\n' != *it)
            return false;
        it = std::next(it);
        if ('\r' != *it)
            return false;

        return true;
    }

    int advance(int n) override {
        if (n == 1) {
            str += c;
        }
        if (state == 0) {
            if (ends_with_two_new_lines()) {
                state = 1;
                header = get_header_from_string(header_string(str));
                content_length = sv_to_size_t(header["Content-Length"]);
                body_str.append(content_length, '0');
            }
        } else {
            position += n;
        }

        return 0;
    }

    const std::string &getBody() const { return body_str; }
    std::string extractBody() { return std::move(body_str); }

    const std::unordered_map<std::string_view, std::string_view> &getHeader() const { return header; }
    std::pair<std::unordered_map<std::string_view, std::string_view>, std::string> extractHeader() {
        return {std::move(header), std::move(str)};
    }
};

struct HttpBufferSender : public CompositeBufferSender {

    HttpBufferSender(const RequestLine &request_line, Buffer_Ptr header_buffer, Buffer_Ptr body_buffer) {
        buffers.emplace_back(new ContainerBufferSender<std::string>(request_line.to_string()));
        buffers.emplace_back(std::move(header_buffer));
        buffers.emplace_back(std::move(body_buffer));
    }
};
