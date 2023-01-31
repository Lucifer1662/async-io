#pragma once
#include "buffer.h"

// Sends the whole container on the wire
// Note there is no corresponding receiver because there is no general method to deduce how long this message is
// But is good for a reusable way to send an already formatted piece of data
// Container must have .data(), and .size()
template <typename Container> struct ContainerBufferSender : public Buffer {
    Container container;
    size_t position = 0;
    ContainerBufferSender(Container container) : container(std::move(container)) {}
    ContainerBufferSender(ContainerBufferSender &&sender) = delete;
    ContainerBufferSender(const ContainerBufferSender &sender) = delete;

    std::pair<char *, int> contiguous() override {
        return {(char *)container.data() + position, (int)container.size() - position};
    }

    int advance(int n) override {
        position += n;
        return 0;
    }

    size_t payload_sent_so_far() { return position; }

    void reset() { position = 0; }
};
