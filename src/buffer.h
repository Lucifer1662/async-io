#pragma once
#include <string>

struct Segment
{
    char *data;
    int size;
    bool owner;

    Segment(char *data, int size, bool owner = false)
        : data(data), size(size), owner(owner) {}

    Segment(const Segment &s)
        : owner(s.owner), size(s.size)
    {
        if (s.owner)
        {
            if (s.data && s.size)
            {
                memcpy(data, s.data, s.size);
            }
            else
            {
                data = 0;
            }
        }
        else
        {
            data = s.data;
        }
    }

    Segment(Segment &&s)
        : owner(s.owner), size(s.size), data(s.data)
    {
        s.data = 0;
        s.size = 0;
        s.owner = false;
    }

    ~Segment()
    {
        if (owner && size && data)
        {
            free(data);
        }
    }
};

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
