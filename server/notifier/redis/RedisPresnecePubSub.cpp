#include "RedisPresencePubSub.hpp"

RedisPresencePubSub::RedisPresencePubSub(const std::string& uri) : redis(uri) {}

void RedisPresencePubSub::startSubscriber(const std::string& channel, std::function<void(const std::string&)> callback) {
    std::thread([this,channel,callback]() {
        while(true) {
            try {
                auto sub = redis.subscriber();
                sub.subscribe(channel);
                sub.on_message([callback](std::string ch, std::string msg) {callback(msg); });
                while(true) {
                    sub.consume();
                }
            }catch(const std::exception& e) {
                std::cerr << "Pub/Sub reconnecting"  << e.what() << std::endl;
            }
        }
    }).detach();
}

void RedisPresencePubSub::markOnline(const std::string& username, int socket) {
    std::cout << "Try To Make Online in Redise" << std::endl;
    redis.set("user:socket:" + username,std::to_string(socket), std::chrono::hours(2));
    std::cout << "Redis Seted" << std::endl;
    redis.sadd("onlie_users",username);
    std::cout << "Redis Added" << std::endl;
}
void RedisPresencePubSub::markOffline(const std::string& username) {
    redis.del("user:socket:" + username);
    redis.srem("online_users", username);
}

bool RedisPresencePubSub::isOnline(const std::string& username) {
    return redis.sismember("online_users", username);
}


int RedisPresencePubSub::getUserSocket(const std::string& username) {
    auto val = redis.get("user:socket:" + username);
    return val ? std::stoi(*val) : -1;
}

void RedisPresencePubSub::publishMessage(const std::string& channel, const std::string& message) {
    redis.publish(channel,message);
} 

void RedisPresencePubSub::subscribeToChannel(const std::string& channel, std::function<void(const std::string&)> callback) {
    startSubscriber(channel,callback);
}