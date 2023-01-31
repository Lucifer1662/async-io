#pragma once
#include "buffer.h"
#include <string>
#include <algorithm>
#include "ContainerBufferSender.h"

struct CStringBufferReceiver : public Buffer {
    std::string str = "";
    int position = 0;
    char c;
    bool finished = false;

    std::pair<char *, int> contiguous() override { return {&c, str.empty() ? 1 : (finished ? 0 : 1)}; }

    int advance(int n) override {
        if (n == 1) {
            if (c != 0)
                str += c;
            else
                finished = true;

            position += n;
        }
        return 0;
    }

    const std::string &getString() const { return str; }

    std::string extractString() { return std::move(str); }
};

struct CStringBatchBufferReceiver : public Buffer {
    mutable std::string str = "";
    int position = 0;
    char c;
    bool finished = false;
    int batch_size;

    CStringBatchBufferReceiver(int batch_size)
        : batch_size(batch_size) {}

    std::pair<char *, int> contiguous() override {
        if (finished) {
            return {0, 0};
        } else {
            str.append(batch_size, '0');
            return {(char *)str.c_str() + position, batch_size};
        }
    }

    int advance(int n) override {
        auto it = std::find(str.begin() + position, str.end(), 0);

        // found null character
        if (it != str.end()) {
            finished = true;
            auto num_used = std::distance(str.begin() + position, it) + 1;
            position += num_used;
            auto unused = n - num_used;
            if (unused == 0)
                return -1;
            return unused;
        } else {
            position += n;
        }

        return 0;
    }

    const std::string &getString() const {
        str.resize(position - 1);
        return str;
    }

    std::string extractString() {
        str.resize(position - 1);
        return std::move(str);
    }
};

struct CStringBufferSender : public Buffer {
    std::string str;
    size_t position = 0;
    CStringBufferSender(std::string str)
        : str(std::move(str)) {}

    std::pair<char *, int> contiguous() override {
        return {(char *)str.c_str() + position, (int)str.size() + 1 - position};
    }

    int advance(int n) override {
        position += n;
        return 0;
    }

    size_t payload_sent_so_far() { return position; }

    void reset() { position = 0; }
};
