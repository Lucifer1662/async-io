#pragma once
#include "buffer.h"
#include <string>


struct CStringBufferReceiver : public Buffer
{
    std::string str = "";
    int position = 0;
    char c;

    std::pair<char *, int> contiguous() override
    {
        return {&c, str.empty() ? 1 : (str.back() == 0 ? 0 : 1)};
    }

    void advance(int n) override
    {
        str += c;
        position += n;
    }

    const std::string &getString() const
    {
        return str;
    }

    std::string extractString()
    {
        return std::move(str);
    }

    ~CStringBufferReceiver() {}
};

struct CStringBufferSender : public Buffer
{
    std::string str;
    int position = 0;
    CStringBufferSender(std::string str)
        : str(str) {}

    std::pair<char *, int> contiguous() override
    {   
        return {(char*)str.c_str() + position, (int)str.size() + 1 - position};
    }

    void advance(int n) override
    {
        position += n;
    }

    ~CStringBufferSender() {}
};
