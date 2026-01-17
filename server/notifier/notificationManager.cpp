#include "notificationManager.hpp"
#include <sys/socket.h>

RealTimeManager RealTimeManager::instance;

void RealTimeManager::setPresence(std::unique_ptr<IPresencePubSub> p ) {
    presence = std::move(p);
}

void RealTimeManager::markOnline(const std::string& username, int socket) {
    presence->markOnline(username,socket);
    socketToUser[socket] = username;
}

void RealTimeManager::markOffline(const std::string& username) {
    presence->markOffline(username);
}

void RealTimeManager::unregisterUserBySocket(int socket) {
    auto it = socketToUser.find(socket);
    if (it != socketToUser.end()) {
        presence->markOffline(it->second);
        socketToUser.erase(it);
    }
}

bool RealTimeManager::isOnline(const std::string& username) {
    return presence->isOnline(username);
}

int RealTimeManager::getUserSocket(const std::string& username) {
    return presence->getUserSocket(username);
}

void RealTimeManager::publishMessage(const std::string& channel, const std::string& message) {
    presence->publishMessage(channel,message);
}

void RealTimeManager::subscribeToChannel(const std::string& username, int socket) {
    presence->subscribeToChannel("msg:user" + username, [socket](const std::string& msg) {
            uint32_t len = htonl(msg.size());
            send(socket, &len, sizeof(len), 0);
            send(socket, msg.c_str(), msg.size(), 0);
    });
}