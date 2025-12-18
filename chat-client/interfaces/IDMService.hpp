#ifndef IDM_SERVICE_HPP
#define IDM_SERVICE_HPP

#include "../../shared/messages/messages.hpp"
#include "../../shared/dm/dm.hpp"
#include <vector>

struct Message;
struct DMConversation;

class IDMService {
public:
    virtual ~IDMService() = default;
    
    virtual bool checkUserExists(const std::string& username) = 0;
    virtual std::vector<Message> fetchMessages(const std::string& user, int limit) = 0;
    virtual bool sendMessage(const std::string& toUser,const std::string& content) = 0;
    virtual std::vector<DMConversation> getAllConversations() = 0;
};

#endif // IDM_SERVICE_HPP
