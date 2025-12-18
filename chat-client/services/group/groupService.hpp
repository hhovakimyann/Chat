#ifndef GROUP_SERVICE_HPP
#define GROUP_SERVICE_HPP

#include "../../interfaces/IGroupService.hpp"
#include "../../interfaces/INetwork.hpp"
#include "../../../shared/session/userSession.hpp"

class ClientGroupService : public IGroupService {
private:
    INetwork* network;
    UserSession& session;
public:
    ClientGroupService(INetwork* net, UserSession& s); //++

    std::vector<std::string> listGroups() override; // ++
    bool createGroup(const std::string& name) override; // ++
    bool joinGroup(const std::string& name) override; // ++
    bool leaveGroup(const std::string& name) override; // ++

    bool sendMessage(const std::string& groupName,const std::string& content) override; // ++
    std::vector<Message> fetchMessages(const std::string& groupName, int limit) override; // ++
};

#endif
