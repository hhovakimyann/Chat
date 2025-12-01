#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <iostream>
#include <mysql/mysql.h>

class Connection {
private:
    MYSQL *connection;
    MYSQL_RES *result;
    MYSQL_ROW row;
    bool connected;
public:
    Connection();
    ~Connection();

    void connect();
    void close();
    void free_res();

    void reconnect(); 
    bool isConnected() const;

    MYSQL* getConnection();
    MYSQL_RES* getRes();
    MYSQL_ROW getRow();
};
#endif // CONNECTION HPP