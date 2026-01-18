#ifndef REDIS_PUB_SUB
#define REDIS_PUB_SUB

#include "../../interfaces/IPresencePubSub.hpp"
#include <sw/redis++/redis++.h>
#include <iostream>
#include <chrono>
#include <thread>

class RedisPresencePubSub : public IPresencePubSub {
private:
    sw::redis::Redis redis;
    void startSubscriber(const std::string& channel, std::function<void(const std::string&)> callback);

public:
    explicit RedisPresencePubSub(const std::string& uri = "tcp://127.0.0.1:6379");
    virtual ~RedisPresencePubSub() = default;

    void markOnline(const std::string& username, int socket) override;
    void markOffline(const std::string& username) override;
    bool isOnline(const std::string& username) override;
    int getUserSocket(const std::string& username) override;
    void publishMessage(const std::string& channel, const std::string& message) override;
    void subscribeToChannel(const std::string& channel,std::function<void(const std::string&)> callback) override;
};

#endif // REDIS_PUB_SUB


