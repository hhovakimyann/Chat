#ifndef SERVER_HPP
#define SERVER_HPP

#include <poll.h>
#include <map>
#include <sys/socket.h>
#include <netinet/in.h>
#include "utils/threadPool/threadPool.hpp"

class RequestRouter;

class Server {
private:
    int server_fd;
    sockaddr_in address {};
    std::mutex mtx;
    std::atomic<bool> serverRunning{true};
    RequestRouter& router;
    
    ThreadPool threadPool;
    std::vector<struct pollfd> poll_fds;
    std::map<int, std::string> clientBuffers;

public:
    Server(RequestRouter &r);
    ~Server();

    bool start(int port = 8080);
    void stop();
    bool isRunning() const;

private:
   void handleRequest(int client_socket, std::string request);
};

#endif // SERVER_HPP
