#include "groupService.hpp"
#include "../../../nlohmann/json.hpp"
#include <iostream>

ClientGroupService::ClientGroupService(INetwork* net, UserSession& s, IAuthService* auth) : network(net), session(s), authService(auth) {}

std::vector<std::string> ClientGroupService::listGroups() {
    if(!session.isLoggedIn()) return {};
    nlohmann::json req_json {
        {"action","list_groups"},
        {"token", session.getAccessToken()}
    };

    std::string req = req_json.dump() + "\n";
    if (!network->sendData(req)) return {};
    
    std::string response_str = network->receiveData();
    try {
        auto resp = nlohmann::json::parse(response_str);

        if (resp.value("status", "") != "success") {
             if (resp.value("message", "") == "Token expired") {
                if (authService->performRefresh()) {
                    return listGroups(); 
                }
            }
            return {};
        }

        std::vector<std::string> groups;
        for (const auto& g : resp.value("groups", nlohmann::json::array())) {
            groups.emplace_back(g.get<std::string>());
        }
        return groups;

    } catch (...) {
        return {}; 
    }
}

bool ClientGroupService::createGroup(const std::string& name) {
    if (!session.isLoggedIn()) return false;

    nlohmann::json req_json = {
        {"action", "create_group"},
        {"token", session.getAccessToken()},
        {"name", name}
    };
    std::string req = req_json.dump() + "\n";
    if (!network->sendData(req)) return false;

    std::string response_str = network->receiveData();
    try {
        auto resp = nlohmann::json::parse(response_str);
        if (resp.value("status", "") == "success") return true;

        if (resp.value("message", "") == "Token expired") {
             if (authService->performRefresh()) {
                 return createGroup(name);
             }
        }
        return false;
    } catch (...) {
        return false;
    }
}

bool ClientGroupService::joinGroup(const std::string& name) {
    if (!session.isLoggedIn()) return false;

    nlohmann::json req_json = {
        {"action", "join_group"},
        {"token", session.getAccessToken()},
        {"group_name", name}
    };
    std::string req = req_json.dump() + "\n";
    if (!network->sendData(req)) return false;

    std::string response_str = network->receiveData();
    try {
        auto resp = nlohmann::json::parse(response_str);
        if (resp.value("status", "") == "success") return true;

        if (resp.value("message", "") == "Token expired") {
             if (authService->performRefresh()) {
                 return joinGroup(name);
             }
        }
        return false;
    } catch (...) {
        return false;
    }
}

bool ClientGroupService::leaveGroup(const std::string& name) {
    if (!session.isLoggedIn()) return false;

    nlohmann::json req_json = {
        {"action", "leave_group"},
        {"token", session.getAccessToken()},
        {"group_name", name}
    };
    std::string req = req_json.dump() + "\n";
    if (!network->sendData(req)) return false;

    std::string response_str = network->receiveData();
    try {
        auto resp = nlohmann::json::parse(response_str);
        if (resp.value("status", "") == "success") return true;

        if (resp.value("message", "") == "Token expired") {
             if (authService->performRefresh()) {
                 return leaveGroup(name);
             }
        }
        return false;
    } catch (...) {
        return false;
    }
}

bool ClientGroupService::sendMessage(const std::string& groupName, const std::string & content) {
    if (!session.isLoggedIn() || content.empty()) return false;

    nlohmann::json req_json = {
        {"action", "send_group_message"},
        {"token", session.getAccessToken()},
        {"group_name", groupName},
        {"content", content}
    };

    std::string req = req_json.dump() + "\n";
    if (!network->sendData(req)) return false;

    std::string response_str = network->receiveData();
    try {
        auto resp = nlohmann::json::parse(response_str);
        if (resp.value("status", "") == "success") return true;

        if (resp.value("message", "") == "Token expired") {
             if (authService->performRefresh()) {
                 return sendMessage(groupName, content);
             }
        }
        return false;
    } catch (...) {
        return false;
    }
}

std::vector<Message> ClientGroupService::fetchMessages(const std::string& groupName , int limit) {
    std::vector<Message> messages;
    std::cout << "Starting Fetch Messages" << std::endl;
    if (!session.isLoggedIn() || groupName.empty()) return messages;

    nlohmann::json req_json = {
        {"action", "fetch_group_messages"},
        {"token", session.getAccessToken()},
        {"group_name", groupName},
        {"limit", limit <= 0 ? 50 : limit}
    };
    
    std::string req = req_json.dump() + "\n";
    if (!network->sendData(req)) return messages;

    std::string response_str = network->receiveData();
    try {
        auto resp = nlohmann::json::parse(response_str);

        if (resp.value("status", "") != "success") {
            if (resp.value("message", "") == "Token expired") {
                 if (authService->performRefresh()) {
                     return fetchMessages(groupName, limit);
                 }
            }
            return messages;
        }

        auto msgs = resp.value("messages", nlohmann::json::array());
        for (const auto& m : msgs) {
            std::cout << "Starting Loaing Data" << std::endl;
            Message msg;
            msg.sender = m.value("sender", "unknown");
            std::cout << "1" << std::endl;
            msg.content = m.value("content", "");
            std::cout << "2" << std::endl;
            msg.timestamp = m.value("timestamp", "1970-01-01 00:00:00");
            std::cout << "3" << std::endl;
            messages.push_back(msg);
            std::cout << "4" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "JSON parse error in fetchMessages: " << e.what() << std::endl;
    }

    return messages;
}