#pragma once
#include "../../interfaces/IDMService.hpp"
#include "../../../database/IDatabase.hpp"
#include <vector>

class DMService : public IDMService {
private:
    IDatabase& db;
    std::optional<int> getUserId(const std::string& username);
    
public:
    explicit DMService(IDatabase& database);

    std::optional<int> checkUserExists(const std::string& username) override;
    bool sendMessage(const std::string& fromUsername,
                     const std::string& toUsername,
                     const std::string& content) override;

    std::vector<Message> getConversation(const std::string& username1, const std::string& username2, int limit = 50) override;

    std::vector<std::string> getConversationPartners(const std::string& username) override;
};