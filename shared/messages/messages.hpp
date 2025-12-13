#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>

struct Message {
    std::string sender;
    std::string content;
    std::string timestamp;
};

#endif //MESSAGE_HPP