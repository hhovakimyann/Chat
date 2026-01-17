#include "notifier/notificationManager.hpp"
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
    fcntl(server_fd, F_SETFL, O_NONBLOCK);
    
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) < 0) {
        std::cerr << "Bind failed on port " << port << "\n";
        return false;
    }

    if (listen(server_fd, 1024) < 0) { 
        std::cerr << "Listen failed\n";
        return false;
    }

    std::cout << "Server listening on port " << port << " (Pulse I/O Mode)...\n";

    struct pollfd server_poll;
    server_poll.fd = server_fd;
    server_poll.events = POLLIN;
    poll_fds.push_back(server_poll);

    while (serverRunning) {
        int ret = poll(poll_fds.data(), poll_fds.size(), -1); 
        if (ret < 0) {
            std::cerr << "Poll failed\n";
            continue; 
        }

        for (size_t i = 0; i < poll_fds.size(); ++i) {
            if (poll_fds[i].revents == 0) continue;

            if (poll_fds[i].fd == server_fd) {
                 if (poll_fds[i].revents & POLLIN) {
                    sockaddr_in clientAddr;
                    socklen_t clientLen = sizeof(clientAddr);
                    int clientSocket = accept(server_fd, (struct sockaddr*)&clientAddr, &clientLen);
                    
                    if (clientSocket >= 0) {
                        fcntl(clientSocket, F_SETFL, O_NONBLOCK);
                        std::cout << "Connected Client with Socket Id " << clientSocket << std::endl;
                        
                        struct pollfd client_poll;
                        client_poll.fd = clientSocket;
                        client_poll.events = POLLIN;
                        poll_fds.push_back(client_poll);
                        clientBuffers[clientSocket] = "";
                    } else {
                        if (errno != EWOULDBLOCK && errno != EAGAIN) {
                             std::cerr << "Accept failed\n";
                        }
                    }
                 }
            } else {
                if (poll_fds[i].revents & POLLIN) {
                    char buffer[4096];
                    ssize_t bytes = recv(poll_fds[i].fd, buffer, sizeof(buffer), 0);

                    if (bytes <= 0) {
                        if (bytes == 0 || (errno != EWOULDBLOCK && errno != EAGAIN)) {
                            int fd = poll_fds[i].fd;
                            std::cout << "Client " << fd << " disconnected" << std::endl;
                            RealTimeManager::getInstance().unregisterUserBySocket(fd);
                            close(fd);
                            clientBuffers.erase(fd);
                            poll_fds.erase(poll_fds.begin() + i);
                            --i; 
                        }
                    } else {
                         int fd = poll_fds[i].fd;
                         clientBuffers[fd].append(buffer, bytes);
                         
                         while (true) {
                             if (clientBuffers[fd].size() < 4) break;

                             uint32_t msgLenNetwork;
                             std::memcpy(&msgLenNetwork, clientBuffers[fd].data(), 4);
                             uint32_t msgLen = ntohl(msgLenNetwork);

                             if (msgLen > 10 * 1024 * 1024) { 
                                 std::cerr << "Invalid message length. Disconnecting " << fd << std::endl;
                                 RealTimeManager::getInstance().unregisterUserBySocket(fd);
                                 close(fd);
                                 clientBuffers.erase(fd);
                                 poll_fds.erase(poll_fds.begin() + i);
                                 --i; 
                                 break;
                             }

                             if (clientBuffers[fd].size() < 4 + msgLen) break;

                             std::string request = clientBuffers[fd].substr(4, msgLen);
                             clientBuffers[fd].erase(0, 4 + msgLen);

                             threadPool.enqueue([this, fd, request] {
                                 handleRequest(fd, request);
                             });
                         }
                    }
                }
            }
        }
    }

    return true;
}

void Server::stop() {
    serverRunning = false;

    if (server_fd >= 0) {
        close(server_fd);
        server_fd = -1;
    }
    for (size_t i = 1; i < poll_fds.size(); ++i) {
         RealTimeManager::getInstance().unregisterUserBySocket(poll_fds[i].fd);
         close(poll_fds[i].fd);
    }
    poll_fds.clear();

    std::cout << "Server stopped.\n";
}

void Server::handleRequest(int clientSocket, std::string request) {
    std::cout << "Received request from client " << clientSocket << std::endl;
    std::string response = router.handle(request,clientSocket);
    
    std::cout << "Generated response size: " << response.size() << std::endl;

    uint32_t responseLen = static_cast<uint32_t>(response.size());
    uint32_t responseLenNetwork = htonl(responseLen);
    
    auto sendAll = [clientSocket](const void* data, size_t len) -> bool {
        const char* ptr = static_cast<const char*>(data);
        size_t totalSent = 0;
        int maxRetries = 50;

        while (totalSent < len) {
            ssize_t sent = send(clientSocket, ptr + totalSent, len - totalSent, 0);
            if (sent < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                    if(maxRetries-- <= 0) return false;
                    continue;
                }
                return false;
            }
            totalSent += sent;
        }
        return true;
    };

    if (!sendAll(&responseLenNetwork, sizeof(responseLenNetwork))) {
        std::cerr << "Failed to send response length to " << clientSocket << std::endl;
        return;
    }
    
    if (!sendAll(response.c_str(), response.size())) {
        std::cerr << "Failed to send response content to " << clientSocket << std::endl;
    }
}

bool Server::isRunning() const {
    return serverRunning;
}