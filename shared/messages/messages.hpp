#ifndef MESSAGES_HPP
#define MESSAGES_HPP

#include <string>

struct Message {
    std::string sender;
    std::string content;
    std::string timestamp;
};

#endif