#ifndef INPUT_CHECKERS
#define INPUT_CHECKERS

#include <iostream>


class Checker {
private:
public:
    bool checkUsername(const std::string &name);
    bool checkName(const std::string &name);
    bool checkSurname(const std::string& surname);
    bool checkEmail(const std::string& email);
    bool checkPassword(const std::string& password);
};

#endif // INPUT_CHECKERS