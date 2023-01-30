#pragma once
#include "buffer.h"
#include <vector>
#include "endian.h"

class TypeLengthBuffer : public Buffer
{
    char type;
    size_t length;
    std::vector<char> data;
    int state = 0;
    bool isReceiver = true;
    int sender_pos = 0;

    void mode(bool is_receiver)
    {
        isReceiver = is_receiver;
    }

    std::pair<char *, int> contiguous() override
    {
        if (state == 0)
        {
            return {&type, 1};
        }
        else if (state == 1)
        {
            return {(char *)&length, sizeof(size_t)};
        }
        else
        {
            if (isReceiver)
            {
                return {&data[data.size()], data.capacity() - data.size()};
            }
            else
            {
                return {&data[sender_pos], data.size() - sender_pos};
            }
        }
    }

    int advance(int n) override
    {
        if (state == 0)
        {
            state = 1;
        }
        else
        {
            if (isReceiver)
            {
                data.reserve(length);
            }
            else
            {
                sender_pos += n;
            }
            state = 2;
        }
        return 0;
    }
};

class TypeLengthBufferReceiver : public Buffer
{
    char type;
    size_t length;
    std::vector<char> data;
    int state = 0;
    size_t received_data = 0;
public:

    std::pair<char *, int> contiguous() override
    {
        if (state == 0)
        {
            return {&type, 1};
        }
        else if (state >= 1 && state <= sizeof(size_t))
        {
            auto sent_length = (state-1);
            return {((char *)&length) + sent_length , sizeof(size_t) - sent_length };
        }
        else
        {
            return {data.data() + received_data, data.size() - received_data};
        }
    }

    int advance(int n) override
    {
        if (state == 0)
        {
            state = 1;
        }else if (state >= 1 && state <= sizeof(size_t)+1){
            state += n;
            if(state == sizeof(size_t)+1){
                data.assign(network_to_host64(length), 0);
                state ++;
            }
        }
        else
        {
            received_data += n;
        }
        return 0;
    }

    char get_type() { return type; }
    size_t get_size() { return network_to_host64(length); }
    const std::vector<char> &view_data() const { return data; }
    std::vector<char> take_data() { return std::move(data); }
};

class TypeLengthBufferSender : public Buffer
{
    char type;
    size_t length;
    std::vector<char> data;
    int state = 0;
    int sender_pos = 0;
public:

    TypeLengthBufferSender(char type, std::vector<char> data)
        : type(type), length(host_to_network64(data.size())), data(std::move(data)) {}

    std::pair<char *, int> contiguous() override
    {
        if (state == 0)
        {
            return {&type, 1};
        }
        else if (state >= 1 && state <= 4)
        {
            auto sent_length = (state-1);
            return {((char *)&length) + sent_length , sizeof(size_t) - sent_length };
        }
        else
        {
            return {data.data() + sender_pos, data.size() - sender_pos};
        }
    }

    int advance(int n) override
    {
        if (state == 0)
        {
            state = 1;
        }else if (state >= 1 && state <= 4){
            state += n;
        }
        else
        {
            sender_pos += n;
            state = 2;
        }
        return 0;
    }

    size_t payload_sent_so_far(){
        return sender_pos;
    }
};
