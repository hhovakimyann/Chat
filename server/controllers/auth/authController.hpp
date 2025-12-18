#ifndef AUTH_CONTROLLER_HPP
#define AUTH_CONTROLLER_HPP

#include <string>
#include "../../../nlohmann/json.hpp"
#include "../../interfaces/IAuthService.hpp"
#include <memory>


class AuthController {
private:
    std::unique_ptr<IAuthService> authService;
public:
    explicit AuthController(std::unique_ptr<IAuthService> as);

    nlohmann::json registerUser(const nlohmann::json& req, int clientSocket);
    nlohmann::json login(const nlohmann::json& req, int clientSocket);
};

#endif

