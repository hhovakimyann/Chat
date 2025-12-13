#pragma once

#include "IDBconnection.hpp"
#include "IDatabase.hpp"
#include <mysql/mysql.h>
#include <mutex>

class MySQLConnection : public IDatabase, public IDatabaseConnection<MYSQL_RES,MYSQL,MYSQL_ROW> {
private:
    MYSQL* conn = nullptr;
    std::mutex dbMutex;
    bool connected;

public:
    MySQLConnection();
    ~MySQLConnection();

    void connect() override;
    void close() override;
    bool isConnected() const override;

    bool execute(const std::string& sql) override;
    std::vector<std::vector<std::string>> query(const std::string& sql) override;
    std::string escape(const std::string& str) const override;


    MYSQL* getConnection() const override;
    MYSQL_RES* executeAndReturn(const std::string& sql) override;
    void freeResult(MYSQL_RES* res) override;

    std::string escapeString(const std::string& str) const override { return escape(str); }
    std::size_t getNumRows(MYSQL_RES* res) override;
    MYSQL_ROW getRow(MYSQL_RES* res) override;
};

