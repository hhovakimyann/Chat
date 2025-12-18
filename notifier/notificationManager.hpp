#ifndef NOTIFICATION_MANAGER_HPP
#define NOTIFICATION_MANAGER_HPP

#include <string>
#include <unordered_map>
#include <mutex>
#include <optional>
#include <functional>
#include <vector>
#include <memory>

class NotificationManager {
public:
    static NotificationManager& getInstance();
    
    void registerUser(const std::string& username, int socket);
    void unregisterUser(const std::string& username);
    void unregisterUserBySocket(int socket);
    std::optional<int> getSocket(const std::string& username);
    bool isUserOnline(const std::string& username);
    
    bool sendToUser(const std::string& username, const std::string& message);
    void broadcast(const std::string& message, const std::vector<std::string>& exclude = {});
    
    void forEachUser(std::function<void(const std::string&, int)> callback);
    
private:
    NotificationManager() = default;
    ~NotificationManager() = default;
    NotificationManager(const NotificationManager&) = delete;
    NotificationManager& operator=(const NotificationManager&) = delete;
    
    std::unordered_map<std::string, int> usernameToSocket;
    std::unordered_map<int, std::string> socketToUsername;
    std::mutex mutex;
};

#endif