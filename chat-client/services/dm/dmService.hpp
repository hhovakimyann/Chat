#ifndef DM_SERVICE_HPP
#define DM_SERVICE_HPP

#include "../../interfaces/IDMService.hpp"
#include "../../interfaces/INetwork.hpp"
#include "../../../shared/session/userSession.hpp"
#include "../../interfaces/IAuthService.hpp"

class ClientDMService : public IDMService {
private:
    INetwork* network;
    IAuthService* authService;
    UserSession& session;

public:
    ClientDMService(INetwork* net, UserSession& s, IAuthService* auth);

    bool checkUserExists(const std::string& username) override;
    std::vector<Message> fetchMessages(const std::string& user, int limit) override;
    bool sendMessage(const std::string& toUser,const std::string& content) override;
    std::vector<DMConversation> getAllConversations() override;
};

#endif
