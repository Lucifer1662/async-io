#pragma once
#include <utility>

struct Buffer
{
    // can under quote on number of bytes
    virtual std::pair<char *, int> contiguous()
    {
        return {0, 0};
    }
    virtual void advance(int n) {}

    virtual ~Buffer() {}
};
