#ifndef MESSAGE_HPP
#define MESSAGE_HPP
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <string>
#include <iostream>
class Message{
    public:
         Message() : bodyLength_(0) {}
        
        enum {maxBytes = 512};
        enum {header = 4};

        Message(std::string message){
            bodyLength_ = getNewBodyLength(message.size());
            encodeHeader();
            std::memcpy(data + header, message.c_str(), bodyLength_);
        };

    };
