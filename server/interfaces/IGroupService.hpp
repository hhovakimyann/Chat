#pragma once
#include "../../shared/messages/messages.hpp"
#include <vector>
#include <string>
#include <optional>

struct GroupInfo {
    std::string name;
    std::string creator;
    std::string created_at;
    int member_count;
};

class IGroupService {
public:
    virtual ~IGroupService() = default;

    virtual bool createGroup(const std::string& groupName, const std::string& creatorUsername) = 0;
    virtual bool joinGroup(const std::string& groupName, const std::string& username) = 0;
    virtual bool leaveGroup(const std::string& groupName, const std::string& username) = 0;
    virtual bool addMember(const std::string& groupName, const std::string& adminUsername, const std::string& newUsername) = 0;

    virtual bool sendMessage(const std::string& groupName, const std::string& fromUsername, const std::string& content) = 0;
    virtual std::vector<Message> getMessages(const std::string& groupName, int limit = 50) = 0;

    virtual std::vector<std::string> getUserGroups(const std::string& username) = 0;
    virtual std::vector<std::string> getMembers(const std::string& groupName) = 0;
    virtual std::optional<GroupInfo> getGroupInfo(const std::string& groupName) = 0;
};