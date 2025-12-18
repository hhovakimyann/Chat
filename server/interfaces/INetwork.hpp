#ifndef INETWORK_HPP
#define INETWORK_HPP

#include <string>

class INetwork {
public:
    virtual ~INetwork() = default;

    virtual bool connectServer() = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() = 0;
    virtual bool sendData(const std::string& data) = 0;
    virtual std::string receiveData() = 0;
};

#endif // INETWORK_HPP
