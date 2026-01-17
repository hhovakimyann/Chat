#ifndef SERVER_AUTH_SERVICE_HPP
#define SERVER_AUTH_SERVICE_HPP

#include "../../interfaces/IAuthService.hpp"
#include "../../../database/IDatabase.hpp"
#include "../../password/password.hpp"
#include "../../../shared/session/userSession.hpp"
#include "../../utils/jwt/jwt.hpp"
#include <memory>
#include <string>

class ServerAuthService : public IAuthService {
private:
    IDatabase& db;

    JWT::Tokens generateJWT(int userId, const std::string& username);
    bool usernameExists(const std::string& username); // ++
    bool emailExists(const std::string& email); // ++

public:
    explicit ServerAuthService(IDatabase& connection); // ++

    std::optional<UserInfo> login(const std::string& username, const std::string& password) override; // ++
    std::optional<UserInfo> registerUser(const std::string& username, const std::string& firstName, const std::string& secondName, const std::string& email, const std::string& password) override; // ++
};

#endif