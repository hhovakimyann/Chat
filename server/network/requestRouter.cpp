#include "requestRouter.hpp"

RequestRouter::RequestRouter(AuthController& ac, DMController& dc, GroupController& gc) : authCtrl(ac), dmCtrl(dc), groupCtrl(gc) {

    // === AUTH === 
    handlers["login"] = [this](const nlohmann::json& req, int clientSocket) -> nlohmann::json {
        std::cout << "Login Handler" << std::endl;
        return this->authCtrl.login(req,clientSocket);
    };

    handlers["register"] = [this](const nlohmann::json& req, int clientSocket)  -> nlohmann::json {
        std::cout << "Register Handler" << std::endl;
        return this->authCtrl.registerUser(req,clientSocket);
    };

    handlers["refresh"] = [this](const nlohmann::json& req, [[maybe_unused]] int clientSocket) -> nlohmann::json {
        std::cout << "Refresh Handler" << std::endl;
        return this->authCtrl.refresh(req);
    };

    auto protectedHandler = [](auto handlerFunc) -> Handler {
        return [handlerFunc](const nlohmann::json& req, [[maybe_unused]] int clientSocket) -> nlohmann::json {
            if(!req.contains("token")) {
                return {
                    {"success", "error"},
                    {"message", "Invlaid Data"}
                };
            }
            
            std::string token = req["token"].get<std::string>();

            try {
                auto decode = JWT::decode(token);
                if(!decode) {
                    return {
                        {"status", "error"},
                        {"message", "Invlaide JWT"}
                    };
                }

                auto expClaim = decode->get_payload_claim("exp");
                long long exp = expClaim.as_integer();
                auto now = std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::system_clock::now().time_since_epoch()
                ).count();

                if (now >= exp) {
                    return {{"status", "error"}, {"message", "Token expired"}};
                }
                return handlerFunc(req);

            }catch(const std::exception& e) {
                std::cerr << "JWT Decode Execption" << std::endl;
                return {
                    {"status", "error"},
                    {"message", "Invalide Token"}
                };
            }
        };
    };

    // === DM === 

    handlers["send_dm"] = protectedHandler([this](const nlohmann::json& req) {
        std::cout << "Send Dm Handler" << std::endl;

        return this->dmCtrl.sendDM(req);
    });

    handlers["fetch_dm"] = protectedHandler([ this](const nlohmann::json& req) {
        std::cout << "Fetch Dm Handler" << std::endl;
        return this->dmCtrl.getMessages(req);
    });

    handlers["list_dm_conversations"] = protectedHandler([this] (const nlohmann::json& req) {
        std::cout << "List DM Conversations Handler" << std::endl;
        return this->dmCtrl.getConversations(req);
    });

    handlers["check_user_exists"] = protectedHandler([this](const nlohmann::json& req) {
        std::cout << "Chech User Exists Handler" << std::endl;
        return this->dmCtrl.checkUserExists(req);
    });

    // ==== GROUPS ==== 
    handlers["list_groups"] = protectedHandler([this](const nlohmann::json& req) {
        std::cout << "List Groups Handler" << std::endl;
        return this->groupCtrl.getGroups(req);
    });

    handlers["create_group"] = protectedHandler([this](const nlohmann::json& req) {
        std::cout << "Create Group Handler" << std::endl;

        return this->groupCtrl.createGroup(req);
    });

    handlers["join_group"] = protectedHandler([this](const nlohmann::json& req) {
        std::cout << "Join Group Handler" << std::endl;
        return this->groupCtrl.joinGroup(req);
    });

    handlers["leave_group"] = protectedHandler([this](const nlohmann::json& req) {
        std::cout << "Leave Group Handler" << std::endl;
        return this->groupCtrl.leaveGroup(req);
    });

    handlers["send_group_message"] = protectedHandler([this](const nlohmann::json& req) {
        std::cout << "Send Group Message Handler" << std::endl;
        return this->groupCtrl.sendGroupMessage(req);
    });

    handlers["fetch_group_messages"] = protectedHandler([this](const nlohmann::json& req) {
        std::cout << "Fetch Group Messages Handler" << std::endl;
        return this->groupCtrl.getMessages(req);
    });

    handlers["get_group_members"] = protectedHandler([this](const nlohmann::json& req) {
        std::cout << "Get Group Handler" << std::endl;
        return this->groupCtrl.getMembers(req);
    });

    handlers["get_group_info"] = protectedHandler([this](const nlohmann::json& req) {
        return this->groupCtrl.getGroupInfo(req);
    });
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

    auto it = handlers.find(action);
    if (it == handlers.end()) {
        return nlohmann::json{{"status", "error"}, {"message", "Unknown action: " + action}}.dump();
    }

    try {
        nlohmann::json response = it->second(req,clientSocket);
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
