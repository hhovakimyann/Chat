#ifndef PASSWORD
#define PASSWORD

#include <iostream>
#include <string>
#include <sodium.h>
#include <termios.h>

struct Password {
    static std::string hashPassword(const std::string& password);
    static bool verifyPassword(const std::string& password, const std::string& hashed);
};
#endif //PASSWORD