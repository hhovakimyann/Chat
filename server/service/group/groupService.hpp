#include "../../interfaces/IGroupService.hpp"
#include "../../../database/IDatabase.hpp"

class GroupService : public IGroupService {
private:
    IDatabase& db;

    std::optional<int> getUserId(const std::string& username);
    std::optional<int> getGroupId(const std::string& groupName);
    bool isCreator(int groupId, int userId);

public:
    explicit GroupService(IDatabase& database);

    bool createGroup(const std::string& groupName, const std::string& creatorUsername) override;
    bool joinGroup(const std::string& groupName, const std::string& username) override;
    bool leaveGroup(const std::string& groupName, const std::string& username) override;
    bool addMember(const std::string& groupName, const std::string& adminUsername, const std::string& newUsername) override;

    bool sendMessage(const std::string& groupName, const std::string& fromUsername, const std::string& content) override;
    std::vector<Message> getMessages(const std::string& groupName, int limit = 50) override;

    std::vector<std::string> getUserGroups(const std::string& username) override;
    std::vector<std::string> getMembers(const std::string& groupName) override;
    std::optional<GroupInfo> getGroupInfo(const std::string& groupName) override;
};