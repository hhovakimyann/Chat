#ifndef I_PRESENCE_PUB_SUB_HPP
#define I_PRESENCE_PUB_SUB_HPP

#include <string>
#include <functional>

class IPresencePubSub {
public:
    virtual ~IPresencePubSub() = default;

    virtual void markOnline(const std::string& username, int socket) = 0;
    virtual void markOffline(const std::string& username) = 0;
    virtual bool isOnline(const std::string& username) = 0;
    virtual int getUserSocket(const std::string& username) = 0;

    virtual void publishMessage(const std::string& channel, const std::string& message) = 0;
    virtual void subscribeToChannel(const std::string& channel, std::function<void(const std::string&)> callback) = 0;

};

#endif // I_PRESENCE_PUB_SUB_HPP