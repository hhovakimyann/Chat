#include "password.hpp"

std::string Password::hashPassword(const std::string& password) {
    if (sodium_init() == -1) {
        throw std::runtime_error("Failed to initialize libsodium");
    }

    char hashed[crypto_pwhash_STRBYTES];

    if (crypto_pwhash_str(
            hashed,
            password.c_str(),
            password.size(),
            crypto_pwhash_OPSLIMIT_MODERATE,  
            crypto_pwhash_MEMLIMIT_MODERATE  
        ) != 0) {
        throw std::runtime_error("Out of memory while hashing password");
    }

    return std::string(hashed);
}

bool Password::verifyPassword(const std::string& password, const std::string& hashed) {
    if (crypto_pwhash_str_verify(
            hashed.c_str(),
            password.c_str(),
            password.size()
        ) == 0) {
        return true; 
    }
    return false; 
}

