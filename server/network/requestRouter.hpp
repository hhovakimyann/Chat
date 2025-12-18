#ifndef REQUEST_ROUTER_HPP
#define REQUEST_ROUTER_HPP

#include "../controllers/auth/authController.hpp"
#include "../controllers/dm/dmController.hpp"
#include "../controllers/group/groupController.hpp"
#include "../../nlohmann/json.hpp"
#include "../utils/jwt/jwt.hpp"
#include <string>
#include <map>

class RequestRouter {
private:
    AuthController& authCtrl;
    DMController& dmCtrl;
    GroupController& groupCtrl;

    using HandlerJson = std::function<nlohmann::json(const nlohmann::json&)>;
    std::unordered_map<std::string, HandlerJson> handlers;

    using HandlerJsonSocket = std::function<nlohmann::json(const nlohmann::json&, int)>;
    std::unordered_map<std::string, HandlerJsonSocket> auth;

    bool validateToken(const std::string& token);
public:
    RequestRouter(AuthController& ac, DMController& dc, GroupController& gc);
    std::string handle(const std::string& rawJson, int clientSocket);

};

#endif
