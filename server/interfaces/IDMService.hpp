#ifndef IDM_SERVICE_HPP
#define IDM_SERVICE_HPP

#include "../../shared/messages/messages.hpp"
#include "../../shared/dm/dm.hpp"
#include <vector>
class IDMService {
public:
    virtual ~IDMService() = default;
    
    virtual std::optional<int> checkUserExists(const std::string& username) = 0;
    virtual bool sendMessage(const std::string& fromUsername,
                             const std::string& toUsername,
                             const std::string& content) = 0;
    
    virtual std::vector<Message> getConversation(const std::string& username1,
                                                 const std::string& username2,
                                                 int limit = 50) = 0;

    virtual std::vector<std::string> getConversationPartners(const std::string& username) = 0;
};

#endif