#include "groupController.hpp"
#include <iostream>

GroupController::GroupController(std::unique_ptr<IGroupService> gs) : groupService(std::move(gs)) {}

std::optional<std::string> GroupController::getUsernameFromToken(const std::string& token) {
    if (token.empty()) return std::nullopt;

    try {
        auto decoded = JWT::decode(token);
        if (!decoded) return std::nullopt;

        auto claim = decoded->get_payload_claim("username");
        return claim.as_string();
    } catch (...) {
        return std::nullopt;
    }
}

nlohmann::json GroupController::getGroups(const nlohmann::json& req) {        
    auto username = getUsernameFromToken(req["token"]);
    if(!username) {
        return {{"status", "error"}, {"message", "Invalid Token"}};
    }
    auto groups = groupService->getUserGroups(*username);

    nlohmann::json res;
    res["status"] = "success";
    res["groups"] = nlohmann::json::array();

    for(const auto& group : groups) {
        res["groups"].push_back(group);
    }

    return res;
}

nlohmann::json GroupController::getMessages(const nlohmann::json& req) {
    if(!req.contains("group")) {
        return {
            {"status", "error"},
            {"message", "Invalid Data"}
        };
    }

    auto username = getUsernameFromToken(req["token"]);
    if(!username) {
        return {
            {"status", "error"},
            {"message", "Invalide Token"}
        };
    }
    std::string groupName = req["group"];
    int limit = req.value("limit",50);

    auto messages = groupService->getMessages(groupName,limit);

    nlohmann::json res;
    res["status"] = "success";
    res["messages"] = nlohmann::json::array();
    for(const auto& message : messages) {
        res["messages"].push_back({
            {"sender", message.sender},
            {"content", message.content},
            {"timestamp", message.timestamp}
        });
    }

    return res;
}

nlohmann::json GroupController::sendGroupMessage(const nlohmann::json& req) {
    if(!req.contains("group") || !req.contains("content")) {
        return {
            {"status", "error"},
            {"message", "Missing Filds"}
        };
    }

    auto username = getUsernameFromToken(req["token"]);
    if(!username) {
        return {
            {"status", "error"},
            {"message", "Invalide Token"}
        };
    }

    std::string groupName = req["group"];
    std::string message = req["content"];

    bool success = groupService->sendMessage(groupName,*username,message);

    return success 
        ? nlohmann::json{{"status", "success"}}
        : nlohmann::json{{"status", "error"}, {"message", "Failed to send"}};
}

nlohmann::json GroupController::createGroup(const nlohmann::json& req) {
    if(req.contains("name")) {
        return {
            {"status", "error"},
            {"message", "Invalid Data"}
        };
    }

    auto username = getUsernameFromToken(req["token"]);
    if(!username) {
        return {
            {"status", "error"},
            {"massage", "Invalide Token"}
        };
    }


    std::string groupName = req["token"];
    
    bool success = groupService->joinGroup(groupName,*username);
    return success
        ? nlohmann::json{{"status", "success"}, {"message", "Group created"}}
        : nlohmann::json{{"status", "error"}, {"message", "Group name taken"}};
}

nlohmann::json GroupController::joinGroup(const nlohmann::json& req) {
    if(!req.contains("group")) {
        return {
            {"status", "error"},
            {"message", "Invalide Data"}
        };
    }

    auto username = getUsernameFromToken(req["token"]);
    if(!username) {
        return {
            {"status", "error"},
            {"message", "Invalide Token"}
        };
    }

    std::string groupName = req["group"];
    bool success = groupService->joinGroup(groupName,*username);
    return success
        ? nlohmann::json{{"status", "success"}, {"message", "Joined Group"}}
        : nlohmann::json{{"status", "error"}, {"message", "Failed To Join Group"}};
}

nlohmann::json GroupController::leaveGroup(const nlohmann::json& req) {
    if(!req.contains("group")) {
        return {
            {"status", "error"},
            {"message", "Invalide Data"}
        };
    }

    auto username = getUsernameFromToken(req["token"]);
    if(!username) {
        return {
            {"status", "error"},
            {"message", "Invalide Token"}
        };
    }

    std::string groupName = req["group"];
    bool success = groupService->leaveGroup(groupName,*username);
    return success
        ? nlohmann::json{{"status", "success"}}
        : nlohmann::json{{"status", "error"}, {"message", "Failed To Leave Group"}};
}

nlohmann::json GroupController::addMember(const nlohmann::json& req) {
    if(!req.contains("group") || !req.contains("username")) {
        return {
            {"status","error"},
            {"message", "Invalide Data"}
        };
    }
    
    auto username = getUsernameFromToken(req["token"]);
    if(!username) {
        return {
            {"status", "error"},
            {"message", "Invalide Token"}
        };
    }
    std::string groupName = req["group"];
    std::string newUser = req["username"];

    bool success = groupService->addMember(groupName,*username,newUser);
    return success 
        ? nlohmann::json{{"status", "success"}}
        : nlohmann::json{{"status", "error", "message", "Dailed To Add Member"}};
}

nlohmann::json GroupController::getMembers(const nlohmann::json& req) {
    if(!req.contains("group")) {
        return {
            {"status", "error"},
            {"message", "Invlaide Data"}
        };
    }
    auto username = getUsernameFromToken(req["token"]);
    if(!username) {
        return {
            {"status", "error"},
            {"message", "Invalide Token"}
        };
    }
    std::string groupName = req["group"];
    auto members = groupService->getMembers(groupName);

    nlohmann::json res;
    res["status"] = "success";
    res["members"] = nlohmann::json::array();
    for(const auto& member : members) {
        res["members"].push_back(member);
    }
    return res;
}

nlohmann::json GroupController::getGroupInfo(const nlohmann::json& req) {
    if(!req.contains("group")) {
        return {
            {"status", "error"},
            {"message", "Invalide Data"}
        };
    }
    auto username = getUsernameFromToken(req["token"]);
    if(!username) {
        return {
            {"status", "error"},
            {"message", "Invalide Token"}
        };
    }

    std::string groupName = req["group"];
    auto groupInfo = groupService->getGroupInfo(groupName);

    if(!groupInfo) {
        return {
            {"status", "error"},
            {"message", "Group Not Found"}
        };
    }

    return {
        {"status", "success"},
        {"group", {
            {"name", groupInfo->name},
            {"creator", groupInfo->creator},
            {"created_at", groupInfo->created_at},
            {"member_count", groupInfo->member_count}
        }}
    };
}