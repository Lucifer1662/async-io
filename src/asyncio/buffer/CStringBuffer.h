#pragma once
#include "buffer.h"
#include <string>


struct CStringBufferReceiver : public Buffer
{
    std::string str = "";
    int position = 0;
    char c;
    bool finished = false;

    std::pair<char *, int> contiguous() override
    {
        return {&c, str.empty() ? 1 : (finished ? 0 : 1)};
    }

    void advance(int n) override
    {
        if(c != 0)
            str += c;
        else
            finished = true;

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


struct CStringBufferReceiver : public Buffer
{
    std::string str = "";
    int position = 0;
    char c;
    bool finished = false;
    size_t batch_size;

    CStringBufferReceiver(size_t batch_size):batch_size(batch_size){}

    std::pair<char *, int> contiguous() override
    {
        if(finished){
            return {0,0};
        }else{
            str.append(batch_size, '0');
            return {str.c_str() + position, batch_size}
        }

        return {&c, str.empty() ? 1 : (finished ? 0 : batch_size)};
    }

    void advance(int n, ) override
    {
        if(c != 0)
            str += c;
        else{
            //shrink down over used data
            str.resize(position,'0');
            finished = true;
            return 
        }

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
    size_t position = 0;
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

    size_t payload_sent_so_far(){
        return position;
    }
};
