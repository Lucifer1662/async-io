#include "http_request.h"

std::string_view trim_white_space(std::string_view str) {
    while (!str.empty() && std::isspace(str.front())) {
        str = str.substr(1);
    }

    while (!str.empty() && std::isspace(str.back())) {
        str = str.substr(0, str.size() - 1);
    }
    return str;
}

std::unordered_map<std::string_view, std::string_view> get_header_from_string(std::string_view text) {
    std::unordered_map<std::string_view, std::string_view> header;

    for (;;) {
        auto next_end = text.find("\r\n");
        auto key_value = text.substr(0, next_end);
        if (key_value.empty())
            break;
        auto colon_i = key_value.find_first_of(":");
        auto key = trim_white_space(key_value.substr(0, colon_i));
        auto value = trim_white_space(key_value.substr(colon_i + 1));
        header.insert({key, value});

        // add two for the \r\n
        if (next_end != std::string_view::npos) {
            text = text.substr(next_end + 2);
        } else {
            break;
        }
    }
    return header;
}

std::string_view header_string(std::string_view text) {
    auto end = text.find("\r\n\r\n");
    auto first = text.find("\r\n") + 2;
    auto ret = text.substr(first, end - first);
    return ret;
}

size_t sv_to_size_t(const std::string_view &str) {

    char *endptr;
    char *end = (char *)(str.data() + str.size());
    auto temp = *end;
    *end = 0;

    auto result = strtol(str.data(), &endptr, 10);
    if (*endptr != 0) {
        return 0;
    }
    *end = temp;

    return result;
}

bool ends_with_two_new_lines(const std::string &str) {
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
