#include "authService.hpp"

ClientAuthService::ClientAuthService(INetwork* net, UserSession& s) : network(net), session(s) {}

bool ClientAuthService::checkResponse(const std::string& response_data, UserInfo* outInfo) {
    if (response_data.empty()) {
        std::cerr << "ERROR: Empty response from server." << std::endl;
        return false;
    }

    try {
        nlohmann::json responseJson = nlohmann::json::parse(response_data);
        if(responseJson["status"] != "success") {
            std::cout << "Error: " << responseJson.value("message", "Unknown") << '\n';
            return false;
        }

        std::string jwtSession = responseJson.value("token", "");
        if (jwtSession.empty()) {
            std::cerr << "ERROR: Token missing from successful authorization response." << std::endl;
            return false;
        }
                
        outInfo->firstName = responseJson.value("first_name", "");
        outInfo->secondName = responseJson.value("second_name", ""); 
        outInfo->email = responseJson.value("email", "");                           
        outInfo->jwt = jwtSession; 
        std::cout << "SUCCESS: Authorization data received and stored." << std::endl;
        return true;

    }catch(nlohmann::json::parse_error& error) {
        std::cerr << "ERROR: Failed Pharse JSON " << error.what() << std::endl;
        return false;   
    }
    return false;
}

bool ClientAuthService::login(UserInfo& info, const std::string& password) {
    //=========WORKINGGGG ============
    if(!network->isConnected()) return false;

    nlohmann::json request_json = {
        {"action", "login"}, 
        {"username", info.username},
        {"password", password}
    };

    std::string req = request_json.dump() + "\n";
    if (!network->sendData(req)) return false;
    std::string response_data = network->receiveData();

    if (checkResponse(response_data, &info)) {
        session.login(info);
        return true;
    }
    return false;
}

bool ClientAuthService::registerUser(UserInfo& info, const std::string& password) {
    // ====== WOEKING ==========
    if (!network->isConnected()) return false;
    nlohmann::json request_json = {
        {"action", "register"},
        {"username", info.username},
        {"password", password},
        {"email", info.email},
        {"first_name", info.firstName},
        {"second_name", info.secondName}
    };

    std::string req = request_json.dump() + "\n";
    if (!network->sendData(req)) return false;

    std::string response_data = network->receiveData();

    if (checkResponse(response_data, &info)) {
        session.login(info);
        return true;
    }
    return false;
}