#ifndef SERVER_HPP
#define SERVER_HPP

#include <thread>
#include <vector>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string>

class RequestRouter;

class Server {
private:
    int server_fd;
    sockaddr_in address {};
    std::vector<std::thread> clientThreads;
    std::mutex mtx;
    std::atomic<bool> serverRunning{true};
    RequestRouter& router;

public:
    Server(RequestRouter &r);
    ~Server();

    bool start(int port = 8080);
    void stop();
    bool isRunning() const;

private:
    void handleClient(int client_socket);
};

#endif // SERVER_HPP
