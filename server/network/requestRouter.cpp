#include "requestRouter.hpp"

RequestRouter::RequestRouter(AuthController& ac, DMController& dc, GroupController& gc) : authCtrl(ac), dmCtrl(dc), groupCtrl(gc) {

    // === AUTH === 
    auth["login"] = [this](const nlohmann::json& req, int clientSocket) {
        std::cout << "Login Handler" << std::endl;
    return authCtrl.login(req,clientSocket);
    };

    auth["register"] = [this](const nlohmann::json& req, int clientSocket) {
        std::cout << "Register Handler" << std::endl;

        return authCtrl.registerUser(req,clientSocket);
    };

    // === DM === 

    handlers["send_dm"] = [this](const nlohmann::json& req) {
        std::cout << "Send Dm Handler" << std::endl;

        return dmCtrl.sendDM(req);
    };

    handlers["fetch_dm"] = [this](const nlohmann::json& req) {
        std::cout << "Fetch Dm Handler" << std::endl;
        return dmCtrl.getMessages(req);
    };

    handlers["list_dm_conversations"] = [this] (const nlohmann::json& req) {
        std::cout << "List DM Conversations Handler" << std::endl;
        return dmCtrl.getConversations(req);
    };

    handlers["check_user_exists"] = [this](const nlohmann::json& req) {
        std::cout << "Chech User Exists Handler" << std::endl;
        return dmCtrl.checkUserExists(req);
    };

    // ==== GROUPS ==== 
    handlers["list_groups"] = [this](const nlohmann::json& req) {
        std::cout << "List Groups Handler" << std::endl;
        return groupCtrl.getGroups(req);
    };

    handlers["create_group"] = [this](const nlohmann::json& req) {
        std::cout << "Create Group Handler" << std::endl;

        return groupCtrl.createGroup(req);
    };

    handlers["join_group"] = [this](const nlohmann::json& req) {
        std::cout << "Join Group Handler" << std::endl;
        return groupCtrl.joinGroup(req);
    };

    handlers["leave_group"] = [this](const nlohmann::json& req) {
        std::cout << "Leave Group Handler" << std::endl;
        return groupCtrl.leaveGroup(req);
    };

    handlers["send_group_message"] = [this](const nlohmann::json& req) {
        std::cout << "Send Group Message Handler" << std::endl;
        return groupCtrl.sendGroupMessage(req);
    };

    handlers["fetch_group_messages"] = [this](const nlohmann::json& req) {
        std::cout << "Fetch Group Messages Handler" << std::endl;
        return groupCtrl.getMessages(req);
    };

    handlers["get_group_members"] = [this](const nlohmann::json& req) {
        std::cout << "Get Group Handler" << std::endl;
        return groupCtrl.getMembers(req);
    };

    handlers["get_group_info"] = [this](const nlohmann::json& req) {
        return groupCtrl.getGroupInfo(req);
    };
}

std::string RequestRouter::handle(const std::string& rawJson, int clientSocket) {
    nlohmann::json req;
    
    try {
        req = nlohmann::json::parse(rawJson);
    } catch (const std::exception& e) {
        std::cerr << "JSON parse error: " << e.what() << std::endl;
        return R"({"status":"error","message":"Invalid JSON"})";
    }

    std::string action = req.value("action", "");
    if (action.empty()) {
        return R"({"status":"error","message":"Missing action field"})";
    }

    std::cout << "Routing action: " << action << " (socket: " << clientSocket << ")" << std::endl;

    if(action == "register" || action == "login") {
        auto it = auth.find(action);
        if (it == auth.end()) {
            return nlohmann::json{{"status", "error"}, {"message", "Unknown action: " + action}}.dump();
        }
        try {
            nlohmann::json response = it->second(req, clientSocket);
            return response.dump();
        } catch (const std::exception& e) {
            std::cerr << "Handler error for action " << action << ": " << e.what() << std::endl;
            return nlohmann::json{{"status", "error"}, {"message", "Server error"}}.dump();
        }
    }

    if (!req.contains("token")) {
        return nlohmann::json{
            {"status", "error"},
            {"message", "Missing token"}
        }.dump();
    }
    
    if (!validateToken(req["token"])) {
        return nlohmann::json{
            {"status", "error"},
            {"message", "Invalid or expired token"}
        }.dump();
    }

    auto it = handlers.find(action);
    if (it == handlers.end()) {
        return nlohmann::json{
            {"status", "error"},
            {"message", "Unknown action: " + action}
        }.dump();
    }

    try {
        nlohmann::json response = it->second(req);
        return response.dump();
    } catch (const std::exception& e) {
        std::cerr << "Handler error for action " << action << ": " << e.what() << std::endl;
        return nlohmann::json{
            {"status", "error"},
            {"message", "Server error: " + std::string(e.what())}
        }.dump();
    }
}

bool RequestRouter::validateToken(const std::string& token) {
    try {
        auto decoded = JWT::decode(token);
        return decoded.has_value();
    } catch(const std::exception &e) {
        std::cout << "JWT Decode Error: " << e.what() << std::endl;
        return false;
    }
}
