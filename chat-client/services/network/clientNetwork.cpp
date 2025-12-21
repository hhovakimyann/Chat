#include "clientNetwork.hpp"

ClientNetwork::ClientNetwork() : sock(-1) {
    server_addr.sin_port = htons(8080);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
}

ClientNetwork::~ClientNetwork() {
    disconnect();
}

bool ClientNetwork::connectServer()  {
    sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock < 0) {
        std::cout << "Failed creatoion socket" << std::endl;
        return false;
    }

    if (connect(sock, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Connection to server failed" << std::endl;
        return false;
    }
    
    return true;
}


bool ClientNetwork::sendData(const std::string& data) {
    if(!isConnected()) {
        std::cout << "Socket Dont Connected" << std::endl;
        return false;
    }

    uint32_t len = static_cast<uint32_t>(data.size());
    uint32_t netLen = htonl(len);

    if (send(sock, &netLen, sizeof(netLen), 0) != sizeof(netLen)) {
        std::cerr << "Failed to send length\n";
        disconnect();
        return false;
    }
    
    size_t sent = 0;
    while (sent < data.size()) {
        ssize_t n = send(sock, data.c_str() + sent, data.size() - sent, 0);
        if (n <= 0) {
            std::cerr << "Send failed: " << strerror(errno) << std::endl;
            disconnect();
            return false;
        }
        sent += n;
    }
    return true;
}

std::string ClientNetwork::receiveData() {
    std::lock_guard<std::mutex> lock(receiveMutex);
    if (!isConnected()) {
        std::cout << "Socket Dont Connected" << std::endl;
        return "";
    }

    uint32_t dataLenNetwork = 0;
    ssize_t n = recv(sock, &dataLenNetwork, sizeof(dataLenNetwork), MSG_WAITALL);

    std::cout << "[DEBUG] Received " << n << " bytes for length\n";
    std::cout << "[DEBUG] Raw bytes: ";
    for (int i = 0; i < 4; i++) {
        printf("%02X ", ((unsigned char*)&dataLenNetwork)[i]);
    }
    std::cout << "\n";

    if (n != sizeof(dataLenNetwork)) {
        std::cerr << "Failed to read full length (got " << n << " bytes)\n";
        disconnect();
        return "";
    }

    uint32_t dataLen = ntohl(dataLenNetwork);
    std::cout << "[DEBUG] Parsed length: " << dataLen << "\n";

    if (dataLen == 0) return "";
    if (dataLen > 5'000'000) {
        std::cerr << "Data too large: " << dataLen << "\n";
        disconnect();
        return "";
    }

    std::string buffer;
    buffer.reserve(dataLen);
    char chunk[4096];

    size_t total = 0;
    while (total < dataLen) {
        n = recv(sock, chunk, std::min(sizeof(chunk), dataLen - total), 0);
        if (n <= 0) {
            std::cerr << "Connection closed during receive\n";
            disconnect();
            return "";
        }
        buffer.append(chunk, n);
        total += n;
    }

    return buffer;
}

bool ClientNetwork::isConnected(){
    return sock >= 0;
}

void ClientNetwork::disconnect() {
    if (isConnected()) {
        std::cout << "Called Disconnect" << std::endl;
        close(sock);
        sock = -1;
    }
}