#include "dmService.hpp"
#include <sstream>
#include <iostream>
#include "../../../shared/messages/messages.hpp"
#include <thread>

DMService::DMService(IDatabase& database) : db(database) {
    if(!db.isConnected()) {
        db.connect();
    }
}


std::optional <int> DMService::checkUserExists(const std::string& username) {
    return getUserId(username); 
}
std::optional<int> DMService::getUserId(const std::string& username) {
    if(username.empty()) return std::nullopt;

    std::string esqaped = db.escape(username);
    std::string query = "SELECT id FROM users WHERE username = '" + esqaped + "' LIMIT 1";

    auto rows = db.query(query);
    if(rows.empty()) {
        return std::nullopt;
    }

    try {
        return std::stoi(rows[0][0]);
    }catch(...) {
        return std::nullopt;
    }
}

bool DMService::sendMessage(const std::string& fromUsername, const std::string& toUsername, const std::string& content) {
    if(fromUsername.empty() || toUsername.empty()) {
        std::cout << "[ERROR] Username is empty" << std::endl;
        return false;
    }
    if(fromUsername == toUsername) {
        std::cout << "[ERROR] Cannot send message to self" << std::endl;
        return false;
    }
    if(content.empty()) {
        std::cout << "[ERROR] Message content is empty" << std::endl;
        return false;
    }
   
    auto senderId = getUserId(fromUsername);
    auto receiverId = getUserId(toUsername);
    
    if(!senderId) {
        std::cout << "[ERROR] Sender not found: " << fromUsername << std::endl;
        return false;
    }
    if(!receiverId) {
        std::cout << "[ERROR] Receiver not found: " << toUsername << std::endl;
        return false;
    }
    
    
    std::string escaped;
    try {
        escaped = db.escape(content);
    } catch (const std::exception& e) {
        std::cout << "[ERROR] Failed to escape content: " << e.what() << std::endl;
        return false;
    }
    std::stringstream sql;
    sql << "INSERT INTO direct_message (sender_id, receiver_id, message_text, created_at) "
        << "VALUES (" << *senderId << ", " << *receiverId << ", '" << escaped << "', NOW())";
    
    std::string sql_str = sql.str();
    
    bool success = db.execute(sql_str);
    
    if(success) {
        std::cout << "[SUCCESS] Message sent successfully" << std::endl;
    } else {
        std::cout << "[ERROR] Failed to execute SQL query" << std::endl;
        
    }
    
    return success;
}

std::vector<Message> DMService::getConversation(const std::string& username1, const std::string& username2, int limit) {
    auto id1 = getUserId(username1);
    auto id2 = getUserId(username2);

    if(!id1 || !id2) {
        std::cout << "ERROR: One or both users not found" << std::endl;
        return {};
    }

    std::stringstream sql;
    sql << "SELECT u.username, dm.message_text, dm.created_at "
        << "FROM direct_message dm " 
        << "JOIN users u ON u.id = dm.sender_id "
        << "WHERE (dm.sender_id = " << *id1 << " AND dm.receiver_id = " << *id2 << ") "
        << "OR (dm.sender_id = " << *id2 << " AND dm.receiver_id = " << *id1 << ") "
        << "ORDER BY dm.created_at DESC " 
        << "LIMIT " << limit;

    auto rows = db.query(sql.str());
    
    std::vector<Message> messages;

    for(const auto& row : rows) {
        if (row.size() >= 3) {
            std::string sender = row[0];
            std::string content = row[1];
            std::string timestamp = row[2];
            
            messages.push_back({sender, content, timestamp});
        }
    }
    std::reverse(messages.begin(),messages.end());
    return messages;
}

std::vector<std::string> DMService::getConversationPartners(const std::string& username) {
    auto id = getUserId(username);
    if(!id) return {};


    std::stringstream sql;
    sql << "SELECT DISTINCT u.username " 
        << "FROM direct_message dm "
        << "JOIN users u ON (u.id = dm.sender_id OR u.id = dm.receiver_id) "
        << "WHERE (dm.sender_id = " << *id << " OR dm.receiver_id = " << *id << ") "
        << "AND u.id != " << *id << " "
        << "ORDER BY u.username";
    
    auto rows = db.query(sql.str());
    std::vector<std::string> partners;
    for(const auto& row : rows) {
        if (!row.empty()) {
            partners.push_back(row[0]);
        }
    }

    return partners;
}
