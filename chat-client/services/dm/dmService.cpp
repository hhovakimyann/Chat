#include "dmService.hpp"
#include <iostream>
#include "../../../nlohmann/json.hpp"

ClientDMService::ClientDMService(INetwork *net, UserSession& s) : network(net), session(s) {}

bool ClientDMService::checkUserExists(const std::string& username) {
    nlohmann::json req_json = {
        {"action", "check_user_exists"},
        {"token", session.getJWT()},
        {"username", username}
    };
    std::string req = req_json.dump() + "\n";
    network->sendData(req);
    
    std::string response_str = network->receiveData();
    auto resp = nlohmann::json::parse(response_str);
    
    return resp["status"] == "success" && resp.value("exists", false);
}

std::vector<Message> ClientDMService::fetchMessages(const std::string& withUsername, int limit) {
    std::cout << "[DEBUG] fetchMessages called for user: " << withUsername << std::endl;
    
    nlohmann::json req_json = {
        {"action", "fetch_dm"},
        {"token", session.getJWT()},
        {"with", withUsername},
        {"limit", limit}
    };
    
    std::string req = req_json.dump() + "\n";
    network->sendData(req);
    
    std::string response_str = network->receiveData();
    if (response_str.empty()) return {};

    std::vector<Message> messages;

    try {
        auto resp = nlohmann::json::parse(response_str);

        if (resp["status"] == "success") {
            if (resp.contains("messages") && resp["messages"].is_array()) {
                const auto& msgs_array = resp["messages"];  
                messages.reserve(msgs_array.size());        

                for (size_t i = 0; i < msgs_array.size(); ++i) {
                    const auto& m = msgs_array[i];
                    Message msg;
                    msg.sender    = m.value("sender", "unknown");
                    msg.content   = m.value("content", "a");
                    msg.timestamp = m.value("timestamp", "1970-01-01 00:00:00");
                    messages.push_back(msg); 
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "JSON parse error in fetchMessages: " << e.what() << std::endl;
    }

    return messages;
}

bool ClientDMService::sendMessage(const std::string& toUser, const std::string& content) {
    if (!session.isLoggedIn()) return false;
    if (toUser.empty() || content.empty()) return false;

    nlohmann::json req_json = {
        {"action", "send_dm"},
        {"token", session.getJWT()},
        {"to", toUser},
        {"content", content}
    };

    std::string req = req_json.dump() + "\n";
    if (!network->sendData(req)) return false;

    try {
        auto resp = nlohmann::json::parse(network->receiveData());
        return resp.value("status", "") == "success";
    } catch (const std::exception& e) {
        std::cerr << "DM send error: " << e.what() << '\n';
        return false;
    }
}
std::vector<DMConversation> ClientDMService::getAllConversations() {
    nlohmann::json req_json = {
        {"action", "list_dm_conversations"},
        {"token", session.getJWT()}
    };
    std::string req = req_json.dump() + "\n";
    network->sendData(req);
    
    std::string response_str = network->receiveData();
    std::cout << "Hello 3 " << response_str << std::endl;
    auto resp = nlohmann::json::parse(response_str);
    std::cout << "Hello 1" << std::endl;
    std::vector<DMConversation> list;
    std::cout << "Hello 2" << std::endl;
    if (resp["status"] == "success") {
        for (auto& c : resp["conversations"]) {
            DMConversation conv;
            
            if (c.is_string()) {
                conv.otherUser = c.get<std::string>();
                conv.lastMessagePreview = "";
                conv.lastTimestamp = "";
            } 
            else if (c.is_object() && c.contains("username")) {
                conv.otherUser = c["username"].get<std::string>();
                conv.lastMessagePreview = c.value("preview", "");
                conv.lastTimestamp = c.value("time", "");
            }
            else if (c.is_object() && c.contains("user")) {
                conv.otherUser = c["user"].get<std::string>();
                conv.lastMessagePreview = c.value("preview", "");
                conv.lastTimestamp = c.value("time", "");
            }
            
            list.push_back(conv);
        }
    }
    
    return list;
}