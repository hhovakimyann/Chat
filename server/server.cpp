#include "../notifier/notificationManager.hpp"
#include "network/requestRouter.hpp"
#include "../config/env_loader.hpp"
#include "server.hpp"
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <fcntl.h>
#include <csignal>
#include <atomic>

Server::Server(RequestRouter &r) : server_fd(-1), router(r) {}

Server::~Server() {
    stop();
}

bool Server::start(int port) {
    EnvLoader::load(".env");

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        std::cerr << "Socket creation failed\n";
        return false;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed on port " << port << "\n";
        return false;
    }

    if (listen(server_fd, 10) < 0) {
        std::cerr << "Listen failed\n";
        return false;
    }

    std::cout << "Server listening on port " << port << "...\n";

    while (serverRunning) {
        sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int clientSocket = accept(server_fd, (struct sockaddr*)&clientAddr, &clientLen);

        if (clientSocket < 0) {
            if (serverRunning) {
                std::cerr << "Accept failed\n";
            }
            continue;
        }
        std::cout << "Connected Client with Socket Id " << clientSocket << std::endl;

        clientThreads.emplace_back([this, clientSocket]() {
            handleClient(clientSocket);
        });
    }

    return true;
}

void Server::stop() {
    serverRunning = false;

    if (server_fd >= 0) {
        shutdown(server_fd, SHUT_RDWR);
        close(server_fd);
        server_fd = -1;
    }

    for (auto& t : clientThreads) {
        if (t.joinable()) {
            t.join();
        }
    }

    std::cout << "Server stopped.\n";
}

void Server::handleClient(int clientSocket) {
    std::cout << "Client with socket id " << clientSocket << std::endl;
    
    while (serverRunning) {
        uint32_t dataLenNetwork;
        ssize_t lenReceived = recv(clientSocket, &dataLenNetwork, sizeof(dataLenNetwork), 0);
        
        if (lenReceived <= 0) {
            if (lenReceived == 0) {
                std::cout << "Client " << clientSocket << " disconnected" << std::endl;
            } else {
                std::cerr << "Error reading from client " << clientSocket << std::endl;
            }
            break;
        }
        
        if (lenReceived != sizeof(dataLenNetwork)) {
            std::cerr << "Incomplete length from client " << clientSocket << std::endl;
            break;
        }
        
        uint32_t dataLen = ntohl(dataLenNetwork);
        
        if (dataLen == 0 || dataLen > 10 * 1024 * 1024) {
            std::cerr << "Invalid data length from client " << clientSocket << ": " << dataLen << std::endl;
            break;
        }
        
        std::vector<char> buffer(dataLen + 1);
        size_t totalReceived = 0;
        
        while (totalReceived < dataLen) {
            ssize_t bytes = recv(clientSocket, buffer.data() + totalReceived, 
                                dataLen - totalReceived, 0);
            
            if (bytes <= 0) {
                std::cerr << "Error reading data from client " << clientSocket << std::endl;
                break;
            }
            
            totalReceived += bytes;
        }
        
        if (totalReceived != dataLen) {
            std::cerr << "Incomplete data from client " << clientSocket << std::endl;
            break;
        }
        
        buffer[dataLen] = '\0';
        std::string request(buffer.data());
        
        std::cout << "Received request from client " << clientSocket << std::endl;
        
        std::string response = router.handle(request,clientSocket);
        
        uint32_t responseLen = htonl(static_cast<uint32_t>(response.size()));
        
        ssize_t sent = send(clientSocket, &responseLen, sizeof(responseLen), 0);
        if (sent != sizeof(responseLen)) {
            std::cerr << "Failed to send response length to client " << clientSocket << std::endl;
            break;
        }
        
        size_t sentData = 0;
        while (sentData < response.size()) {
            ssize_t n = send(clientSocket, response.c_str() + sentData, 
                           response.size() - sentData, 0);
            if (n <= 0) {
                std::cerr << "Failed to send response data to client " << clientSocket << std::endl;
                break;
            }
            sentData += n;
        }
    }
    
    NotificationManager::getInstance().unregisterUserBySocket(clientSocket);
    close(clientSocket);
    std::cout << "Client " << clientSocket << " connection closed" << std::endl;
}

bool Server::isRunning() const {
    return serverRunning;
}