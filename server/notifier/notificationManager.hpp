#ifndef NOTIFICATION_MANAGER_HPP
#define NOTIFICATION_MANAGER_HPP

#include "../interfaces/IPresencePubSub.hpp"
#include <memory>
#include <string>

#include <map>

class RealTimeManager {
private:
    std::unique_ptr<IPresencePubSub> presence;
    static RealTimeManager instance;
    std::map<int, std::string> socketToUser;

    RealTimeManager() = default;
public:

    static RealTimeManager& getInstance() {
        return instance;
    }
    void setPresence(std::unique_ptr<IPresencePubSub> p);

    void markOnline(const std::string& username, int socket);
    void markOffline(const std::string& username);
    void unregisterUserBySocket(int socket);
    bool isOnline(const std::string& username);
    int getUserSocket(const std::string& username);

    void publishMessage(const std::string& channel, const std::string& message);
    void subscribeToChannel(const std::string& username, int socket);
};

#endif