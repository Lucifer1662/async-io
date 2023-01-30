#pragma once
#include <utility>
#include <memory>

struct Buffer
{
    // can under quote on number of bytes
    virtual std::pair<char *, int> contiguous()
    {
        return {0, 0};
    }
    virtual int advance(int n) {return 0;}

    virtual ~Buffer() {}
};

using Buffer_Ptr = std::unique_ptr<Buffer>;

