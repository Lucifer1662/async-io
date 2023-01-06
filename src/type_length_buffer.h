#pragma once
#include "buffer.h"
#include <vector>



class TypeLengthBufferReceiver : public Buffer{
    char type;
    size_t length;
    std::vector<char> data;
    int state = 0;
    bool isReceiver = true;
    int sender_pos = 0;

    void mode(bool is_receiver){
        isReceiver = is_receiver;
    }

    std::pair<char *, int> contiguous() override {
        if(state == 0){
            return {&type, 1};
        }else if(state == 1){
            return {(char*)&length, sizeof(size_t)};
        }else{
            if(isReceiver){
                return {&data[data.size()], data.capacity() - data.size()};
            }else{
                return {&data[sender_pos], data.size() - sender_pos};
            }
        }
    }

    void advance(int n) override {
        if(state == 0){
            state = 1;
        }else{
            if(isReceiver){
                data.reserve(length);
            }else{
                sender_pos += n;
            }
            state = 2;
        }
    }
};


// class TypeLengthBufferReceiver : public Buffer{
//     char type;
//     size_t length;
//     std::vector<char> data;
//     int state = 0;
//     bool isReceiver = true;

    
//     void mode(bool is_receiver){
//         isReceiver = is_receiver;
//     }

//     std::pair<char *, int> contiguous() override {
//         if(state == 0){
//             return {&type, 1};
//         }else if(state == 1){
//             return {(char*)&length, sizeof(size_t)};
//         }else{
//             return {&data[data.size()], data.capacity() - data.size()};
//         }
//     }

//     void advance(int n) override {
//         if(state == 0){
//             state = 1;
//         }else{
//             data.reserve(length);
//             state = 2;
//         }
//     }
// };


