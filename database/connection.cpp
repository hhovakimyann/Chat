#include "connection.hpp"
#include <stdexcept>
#include "../config/env_loader.hpp"

Connection::Connection() : connection(nullptr), result(nullptr), row(nullptr), connected(false) {}
Connection::~Connection() { close(); }

void Connection::connect() {
    if (connected) return;
    
    connection = mysql_init(nullptr);
    if(!connection) {
        throw std::runtime_error("Failed to initialize MySQL");
    }

    EnvLoader::load(".env");

    std::string host = EnvLoader::get("DB_HOST");
    std::string user = EnvLoader::get("DB_USER");
    std::string pass = EnvLoader::get("DB_PASS");
    std::string name = EnvLoader::get("DB_NAME");

    unsigned int timeout = 30;
    mysql_options(connection, MYSQL_OPT_CONNECT_TIMEOUT, &timeout);
 
    if(!mysql_real_connect(connection, "localhost", "root", "artyom", "chatDatabase", 0, nullptr, 0)) {
        std::string error = "Connection error: ";
        error += mysql_error(connection);
        mysql_close(connection);
        connection = nullptr;
        throw std::runtime_error(error);
    }
    
    bool reconnect = 1;
    mysql_options(connection, MYSQL_OPT_RECONNECT, &reconnect);
    
    connected = true;
    std::cout << "Connected to database successfully" << std::endl;
}

void Connection::close() {
    if(connection) {
        mysql_close(connection);
        connection = nullptr;
    }
    connected = false;
    
}

void Connection::free_res() {
    if (result) {
        mysql_free_result(result);
        result = nullptr;
    }
    
    while (mysql_next_result(connection) == 0) {
        MYSQL_RES* tempRes = mysql_store_result(connection);
        if (tempRes) {
            mysql_free_result(tempRes);
        }
    }
}
bool Connection::isConnected() const {
    return connected;
}

MYSQL* Connection::getConnection() { 
    if (!connected || !connection) {
        throw std::runtime_error("Database connection is not available");
    }
    return connection; 
}



MYSQL* Connection::getConnection() { return connection; }
MYSQL_RES* Connection::getRes() {return result; }
MYSQL_ROW Connection::getRow() {return row; }