#ifndef GROUP_CONTROLLER_HPP
#define GROUP_CONTROLLER_HPP

#include "../../interfaces/IGroupService.hpp"
#include "../../utils/jwt/jwt.hpp"
#include "../../../nlohmann/json.hpp"
#include <memory>
#include <string>

class GroupController {
private:
    std::unique_ptr<IGroupService> groupService;
    std::optional<std::string> getUsernameFromToken(const std::string& token);

public:
    explicit GroupController(std::unique_ptr<IGroupService> gs);

    nlohmann::json getGroups(const nlohmann::json& req);
    nlohmann::json getMessages(const nlohmann::json& req);
    nlohmann::json sendGroupMessage(const nlohmann::json& req);

    nlohmann::json createGroup(const nlohmann::json& req);
    nlohmann::json joinGroup(const nlohmann::json& req);
    nlohmann::json leaveGroup(const nlohmann::json& req);
    nlohmann::json addMember(const nlohmann::json& req);
    nlohmann::json getMembers(const nlohmann::json& req);
    nlohmann::json getGroupInfo(const nlohmann::json& req);
};

#endif
