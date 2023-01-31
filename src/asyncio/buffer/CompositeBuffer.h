#pragma once
#include "buffer.h"
#include <vector>

struct CompositeBufferSender : public Buffer {
  protected:
    std::vector<Buffer_Ptr> buffers;
    size_t current_buffer = 0;
    char *current_block = 0;
    int current_block_size = 0;

    CompositeBufferSender(std::vector<Buffer_Ptr> &&buffers) : buffers(std::move(buffers)) {}
    CompositeBufferSender() {}

    std::pair<char *, int> contiguous() override {
        if (current_buffer >= buffers.size()) {
            return {0, 0};
        }

        auto [data, n] = buffers[current_buffer]->contiguous();
        current_block = data;
        current_block_size = n;

        if (n == 0) {
            current_buffer++;
            return contiguous();
        }
        return {data, n};
    }

    int advance(int n) override {
        auto unused = buffers[current_buffer]->advance(n);

        if (unused > 0) {
            current_buffer++;
            for (; current_buffer < buffers.size() && unused > 0;) {
                auto [new_data, new_size] = buffers[current_buffer]->contiguous();
                if (new_size == 0) {
                    current_buffer++;
                } else {
                    auto num_used = current_block_size - unused;
                    auto amount = std::min(unused, new_size);
                    memcpy(new_data, current_block + num_used, amount);
                    unused -= amount;
                }
            }
        }

        return unused;
    }
};
