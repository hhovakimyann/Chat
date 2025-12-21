#ifndef CLINET_NETWORK_HPP
#define CLINET_NETWORK_HPP

#include "../../interfaces/INetwork.hpp"
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <mutex>

class ClientNetwork : public INetwork{
private:
    int sock;
    struct sockaddr_in server_addr;
    std::mutex receiveMutex;
        
public:
    ClientNetwork(); // ++
    ~ClientNetwork(); //++ 
    
    bool connectServer() override; // ++ 
    bool sendData(const std::string& data) override; //++
    std::string receiveData() override; // ++ 
    void disconnect() override; // ++ 
    bool isConnected() override;
    
};
    

#endif // CLIENTNETWORK_HPP

