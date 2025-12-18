#include "notificationManager.hpp"
#include <sys/socket.h>
#include <unistd.h>
#include <iostream>
#include <algorithm>

NotificationManager& NotificationManager::getInstance() {
    static NotificationManager instance;
    return instance;
}


void NotificationManager::registerUser(const std::string& username, int socket) {
    std::lock_guard<std::mutex> lock(mutex);

    auto it = usernameToSocket.find(username);
    if (it != usernameToSocket.end()) {
        int oldSocket = it->second;
        socketToUsername.erase(oldSocket);
    }
    
    auto socketIt = socketToUsername.find(socket);
    if (socketIt != socketToUsername.end()) {
        std::string oldUser = socketIt->second;
        usernameToSocket.erase(oldUser);
    }
    
    usernameToSocket[username] = socket;
    socketToUsername[socket] = username;
    
    std::cout << "[Notification] User registered: " << username 
              << " (socket: " << socket << ")" << std::endl;
}

void NotificationManager::unregisterUserBySocket(int socket) {
    std::lock_guard<std::mutex> lock(mutex);
    auto it = socketToUsername.find(socket);
    if (it != socketToUsername.end()) {
        std::string username = it->second;
        usernameToSocket.erase(username);
        socketToUsername.erase(it);
        std::cout << "[Notification] Socket unregistered: " << socket 
                  << " (user: " << username << ")" << std::endl;
    }
}


std::optional<int> NotificationManager::getSocket(const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex);
    auto it = usernameToSocket.find(username);
    if (it != usernameToSocket.end()) {
        return it->second;
    }
    return std::nullopt;
}

bool NotificationManager::isUserOnline(const std::string& username) {
    std::lock_guard<std::mutex> lock(mutex);
    return usernameToSocket.find(username) != usernameToSocket.end();
}

bool NotificationManager::sendToUser(const std::string& username, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex);
    auto it = usernameToSocket.find(username);
    std::cout << "AAAAAAAAAAAAAAAAA" << std::endl;
    if (it != usernameToSocket.end()) {
        std::cout << "BBBBBBBBBBBBBBBBBB" << std::endl;
        int socket = it->second;
        
        uint32_t dataLen = htonl(static_cast<uint32_t>(message.size()));
        if (send(socket, &dataLen, sizeof(dataLen), 0) != sizeof(dataLen)) {
            socketToUsername.erase(socket);
            usernameToSocket.erase(it);
            return false;
        }
       
        std::cout << "CCCCCCCCCCCCCCCCC" << std::endl;
        size_t totalSent = 0;
        while (totalSent < message.size()) {
            ssize_t sent = send(socket, message.c_str() + totalSent, 
                               message.size() - totalSent, 0);
            if (sent <= 0) {
                socketToUsername.erase(socket);
                usernameToSocket.erase(it);
                return false;
            }
            totalSent += sent;
        }
        std::cout << "DDDDDDDDDDDDDDDDDDD" << std::endl;
        std::cout << "[Notification] Message sent to " << username 
                  << " (" << message.size() << " bytes)" << std::endl;
        return true;
    }
    std::cout << "Notifier Returned False" << std::endl;
    return false;
}


void NotificationManager::broadcast(const std::string& message, 
    const std::vector<std::string>& exclude) {
    std::lock_guard<std::mutex> lock(mutex);

    for (const auto& [username, socket] : usernameToSocket) {

    if (std::find(exclude.begin(), exclude.end(), username) != exclude.end()) {
        continue;
    }

    uint32_t dataLen = htonl(static_cast<uint32_t>(message.size()));

    if (send(socket, &dataLen, sizeof(dataLen), 0) == sizeof(dataLen)) {
        send(socket, message.c_str(), message.size(), 0);
        }
    }
}
