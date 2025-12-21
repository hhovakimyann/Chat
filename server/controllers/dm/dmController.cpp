#include "../../../shared/session/userSession.hpp"
#include "../../../notifier/notificationManager.hpp"
#include "dmController.hpp"
#include <iostream>
#include <sstream>

DMController::DMController(std::unique_ptr<IDMService> ds) : dmService(std::move(ds)) {}

std::optional<std::string> DMController::getUsernameFromToken(const std::string& token) {
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

std::string DMController::getCurrentTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_time_t), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << now_ms.count();
    
    return ss.str();
}

nlohmann::json DMController::sendDM(const nlohmann::json &req) {
    if(!req.contains("to") || !req.contains("content")) {
    return  {
            {"status", "error"},
            {"message", "Missing fields"}
        };
    }

    std::string token = req["token"];
    std::string toUsername = req["to"];
    std::string content = req["content"];

    if(toUsername.empty() || content.empty()) {
        return {
            {"status", "error"},
            {"message", "Invalid Data"}
        };
    }

    
    auto fromUsername = getUsernameFromToken(token);
    if(!fromUsername) {
        return {
            {"status","error"},
            {"message", "Invalide or expired token"}
        };
    }
    
    if (*fromUsername == toUsername) {
        return {
            {"status", "error"},
            {"message", "Cannot send message to yourself"}
        };
    }

    bool success = dmService->sendMessage(*fromUsername,toUsername,content);
    if(success) {
        nlohmann::json pushNotification;
        pushNotification["type"] = "new_message";
        pushNotification["from"] = *fromUsername;
        pushNotification["content"] = content;
        pushNotification["timestamp"] = getCurrentTimestamp();
        
        std::string pushJson = pushNotification.dump();
        if(NotificationManager::getInstance().isUserOnline(toUsername)) {
            bool pushSent = NotificationManager::getInstance().sendToUser(toUsername, pushJson);
            
            if (pushSent) {
                std::cout << "Push notification sent to " << toUsername << std::endl;
            } else {
                std::cout << "User " << toUsername << " is offline or not found" << std::endl;
            }
        }
        return {
            {"status", "success"},
            {"message", "Message Sent"}
        };
    }
    return {
        {"status", "error"},
        {"message", "Failed To Send Message"}
    };
} 

nlohmann::json DMController::checkUserExists(const nlohmann::json& req) {
    if (!req.contains("username")) {
        return {
            {"status", "error"},
            {"message", "Missing username"}
        };
    }
    
    std::string username = req["username"];
    
    auto userId = dmService->checkUserExists(username);
    
    return {
        {"status", "success"},
        {"exists", userId.has_value()}
    };
}

nlohmann::json DMController::getMessages(const nlohmann::json &req) {
    if(!req.contains("with")) {
        return {
            {"status", "error"},
            {"messages", "Missing token or username"}
        };
    }
    std::string token = req["token"];
    std::string userWith = req["with"];
    int limit = req.value("limit",50);

    auto fromUsername = getUsernameFromToken(token);
    if(!fromUsername) {
        return {
            {"status", "error"},
            {"message", "Invalid Token"}
        };
    }

    auto messages = dmService->getConversation(*fromUsername,userWith,limit);

    nlohmann::json result;
    result["status"] = "success";
    result["messages"] = nlohmann::json::array();

    for(const auto& msg : messages) {
        result["messages"].push_back({
            {"sender", msg.sender},
            {"content", msg.content},
            {"timestamp", msg.timestamp}
        });
    }

    return result;
} 


nlohmann::json DMController::getConversations(const nlohmann::json& req) {
    std::string token = req["token"];

    auto fromUsername = getUsernameFromToken(token);

    if(!fromUsername) {
        return {
            {"status", "error"},
            {"message", "Invalid Token"}
        };
    }
    auto partners = dmService->getConversationPartners(*fromUsername);
    
    nlohmann::json result = {
        {"status", "success"},
        {"conversation", nlohmann::json::array()}
    };

    for(const auto& partner : partners) {
        nlohmann::json conv;
        conv["username"] = partner;   
        result["conversations"].push_back(conv);
    }

    return result;
}
 