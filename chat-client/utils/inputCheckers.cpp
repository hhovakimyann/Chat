#include "inputCheckers.hpp"

bool Checker::checkUsername(const std::string &name) {
    return name.size() >= 8;
}

bool Checker::checkName(const std::string &name) {
    if (name.empty()) return false;

    if (!std::isupper(name[0])) return false;

    for (size_t i = 1; i < name.size(); ++i) {
        if (!std::islower(name[i])) return false;
    }

    return true;
}

bool Checker::checkSurname(const std::string& surname) {
    return checkName(surname);
}

bool Checker::checkEmail(const std::string& email) {
    size_t atPos = email.find('@');
    size_t dotPos = email.find('.', atPos);
    if (atPos == std::string::npos || dotPos == std::string::npos) return false;
    if (atPos == 0 || dotPos == email.size() - 1) return false;
    if (dotPos < atPos + 2) return false; 
    if (email.find(' ') != std::string::npos) return false;
    return true;
}


bool Checker::checkPassword(const std::string& password) {
    if (password.length() < 8)
    return false;

    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;
    bool hasSpecial = false;

    for (char ch : password) {
        if (std::isupper(ch)) hasUpper = true;
        else if (std::islower(ch)) hasLower = true;
        else if (std::isdigit(ch)) hasDigit = true;
        else if (std::ispunct(ch)) hasSpecial = true; 
    }

    return hasUpper && hasLower && hasDigit && hasSpecial;
}