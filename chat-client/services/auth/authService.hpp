#ifndef AUTH_SERVICE_HPP
#define AUTH_SERVICE_HPP

#include "../../interfaces/IAuthService.hpp"
#include "../../interfaces/INetwork.hpp"
#include "../../../shared/session/userSession.hpp"
#include "../../../nlohmann/json.hpp"
#include <iostream>

class ClientAuthService : public IAuthService {
private:
    INetwork* network;
    UserSession& session;
public:
    ClientAuthService(INetwork* net,UserSession& s);
    
    bool checkResponse(const std::string& response_data, UserInfo* outInfo);
    bool login(UserInfo& info, const std::string& password) override;
    bool registerUser(UserInfo& info, const std::string& password) override;
    bool performRefresh();

};

#endif // AUTH_SERVICE_HPP
