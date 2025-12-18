#include "groupService.hpp"
#include <sstream>

GroupService::GroupService(IDatabase& database) : db(database) {}

std::optional<int> GroupService::getUserId(const std::string& username) {
    std::string escaped = db.escape(username);
    std::string query = "SELECT id FROM users WHERE username = '" + escaped + "' LIMIT 1";
    auto rows = db.query(query);
    if(rows.empty()) return std::nullopt;
    return std::stoi(rows[0][0]);
}

std::optional<int> GroupService::getGroupId(const std::string& groupName) {
    std::string escaped = db.escape(groupName);
    std::string query = "SELECT id FROM `groups` WHERE name = '" + escaped + "' LIMIT 1";
    auto rows = db.query(query);
    if(rows.empty()) return std::nullopt;
    return std::stoi(rows[0][0]);
}

bool GroupService::isCreator(int groupId, int userId) {
    auto rows = db.query("SELECT 1 FROM `groups` WHERE id = " + std::to_string(groupId) + " AND created_by = " + std::to_string(userId));
    return !rows.empty();
}

bool GroupService::createGroup(const std::string& groupName, const std::string& creatorUser) {
    auto creatorId = getUserId(creatorUser);
    if(!creatorId) return false;

    std::string escaped = db.escape(groupName);
    bool exist = !db.query("SELECT 1 FROM `groups` WHERE name = '" + escaped + "' ").empty();
    if(exist) return false;

    std::string sql = "INSERT INTO `groups` (name created_at) VALUES ('" + escaped + "', " + std::to_string(*creatorId) + ")";
    if(!db.execute(sql)) return false;

    return joinGroup(groupName,creatorUser);
}

bool GroupService::joinGroup(const std::string& groupName, const std::string& username) {
    auto userId = getUserId(groupName);
    auto groupId = getGroupId(username);
    if(!userId || !groupId) return false;

    std::string query = "INSERT INTO group_members (group_id, user_id) VALUES (" + std::to_string(*groupId) + ", " + std::to_string(*userId) + ")";
    return db.execute(query);
}   

bool GroupService::leaveGroup(const std::string& groupName, const std::string& username) {
    auto userId = getUserId(username);
    auto groupId = getGroupId(groupName);

    if(!userId || !groupId) return false;

    std::string query = "DELETE FROM group_members WHERE group_id = " + std::to_string(*groupId) +
                       " AND user_id = " + std::to_string(*userId);
    return db.execute(query);
}

bool GroupService::addMember(const std::string& groupName, const std::string& adminUser, const std::string& newUsername) {
    auto groupId = getGroupId(groupName);
    auto adminId = getUserId(adminUser);
    auto newUserId = getUserId(newUsername);

    if(!groupId || !adminId || !newUserId) return false;
    if(!isCreator(*groupId,*adminId)) return false;

    return joinGroup(groupName,newUsername);
}

bool GroupService::sendMessage(const std::string& groupName, const std::string& fromUsername, const std::string& content) {
    auto groupId = getGroupId(groupName);
    auto senderId = getUserId(fromUsername);

    if(!groupId || !senderId || content.empty()) return false;
    auto check = db.query("SELECT 1 FROM group_members WHERE group_id = " + std::to_string(*groupId) + " AND user_id = " + std::to_string(*senderId));
    if(check.empty()) return false;

    std::string escaped = db.escape(content);
    std::string query = "INSERT INTO group_messages (group_id, sender_id, message_text) VALUES (" + std::to_string(*groupId) + ", " + std::to_string(*senderId) +", '" + escaped + "')";
    return db.execute(query);
}

std::vector<Message> GroupService::getMessages(const std::string& groupName, int limit) {
    auto groupId = getGroupId(groupName);
    if (!groupId) return {};

    std::stringstream query;
    query << "SELECT u.username, gm.message_text, gm.created_at "
        << "FROM group_messages gm "
        << "JOIN users u ON u.id = gm.sender_id "
        << "WHERE gm.group_id = " << *groupId << " "
        << "ORDER BY gm.created_at DESC LIMIT " << limit;

    auto rows = db.query(query.str());
    std::vector<Message> msgs;

    for (const auto& row : rows) {
        Message m;
        m.sender = row[0];
        m.content = row[1];
        m.timestamp = row[2];
        msgs.push_back(m);
    }

    std::reverse(msgs.begin(), msgs.end());
    return msgs;
}

std::vector<std::string> GroupService::getUserGroups(const std::string& username) {
    auto userId = getUserId(username);
    if(!userId) return {};

    auto rows = db.query("SELECT g.name FROM `groups` g JOIN group_members gm ON gm.group_id = g.id WHERE gm.user_id = " + std::to_string(*userId));
    if(rows.empty()) return {};

    std::vector<std::string> users;
    for(const auto& row : rows) {
        users.push_back(row[0]);
    }

    return users;
}

std::vector<std::string> GroupService::getMembers(const std::string& groupName) {
    auto groupId = getGroupId(groupName);
    if (!groupId) return {};

    auto rows = db.query("SELECT u.username FROM users u "
                          "JOIN group_members gm ON gm.user_id = u.id "
                          "WHERE gm.group_id = " + std::to_string(*groupId));

    std::vector<std::string> members;
    for (const auto& row : rows) {
        members.push_back(row[0]);
    }
    return members;
}

std::optional<GroupInfo> GroupService::getGroupInfo(const std::string& groupName) {
    auto groupId = getGroupId(groupName);
    if (!groupId) return std::nullopt;

    auto rows = db.query("SELECT g.name, u.username, g.created_at, "
                          "(SELECT COUNT(*) FROM group_members WHERE group_id = g.id) "
                          "FROM `groups` g "
                          "JOIN users u ON u.id = g.created_by "
                          "WHERE g.id = " + std::to_string(*groupId));

    if (rows.empty()) return std::nullopt;

    GroupInfo info;
    info.name = rows[0][0];
    info.creator = rows[0][1];
    info.created_at = rows[0][2];
    info.member_count = std::stoi(rows[0][3]);
    return info;
}