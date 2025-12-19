#ifndef DM_CONTROLLER_HPP
#define DM_CONTROLLER_HPP

#include "../../interfaces/IDMService.hpp"
#include "../../utils/jwt/jwt.hpp"
#include "../../../nlohmann/json.hpp"
#include <string>
#include <memory>

class DMController {
private:

    std::unique_ptr<IDMService> dmService;
    std::optional<std::string> getUsernameFromToken(const std::string& token);
    std::string getCurrentTimestamp();

public:
    explicit DMController(std::unique_ptr<IDMService> ds);
    nlohmann::json checkUserExists(const nlohmann::json& req);
    nlohmann::json sendDM(const nlohmann::json& req);
    nlohmann::json getMessages(const nlohmann::json& req);
    nlohmann::json getConversations(const nlohmann::json& req);
};

#endif
