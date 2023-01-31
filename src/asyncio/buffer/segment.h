#pragma once

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
