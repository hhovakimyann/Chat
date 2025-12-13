#include "MySQLconnection.hpp"
#include "../config/env_loader.hpp"
#include <iostream>
#include <stdexcept>

MySQLConnection::MySQLConnection() = default;

MySQLConnection::~MySQLConnection() {
    close();
}

void MySQLConnection::connect() {
    std::lock_guard<std::mutex> lock(dbMutex);
    std::cout << "Connection MYSQL Started" << std::endl;
    if (connected) return;

    conn = mysql_init(nullptr);
    if (!conn) throw std::runtime_error("mysql_init failed");

    EnvLoader::load("../.env");
    auto host = EnvLoader::get("DB_HOST");
    auto user = EnvLoader::get("DB_USER");
    auto pass = EnvLoader::get("DB_PASS");
    auto name = EnvLoader::get("DB_NAME");

    mysql_options(conn, MYSQL_INIT_COMMAND, "SET sql_mode='ANSI_QUOTES'");
    if (!mysql_real_connect(conn, host.c_str(), user.c_str(), pass.c_str(),
                            name.c_str(), 0, nullptr, 0)) {
        std::string err = mysql_error(conn);
        mysql_close(conn);
        conn = nullptr;
        throw std::runtime_error("MySQL connect failed: " + err);
    }

    connected = true;
    std::cout << "Connection MYSQL Successed" << std::endl;
    return;
}

void MySQLConnection::close() {
    std::lock_guard<std::mutex> lock(dbMutex);
    if (conn) {
        mysql_close(conn);
        conn = nullptr;
    }
    connected = false;
}

bool MySQLConnection::isConnected() const {
    return connected;
}

bool MySQLConnection::execute(const std::string& sql) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if (!connected) connect();
    if (mysql_query(conn, sql.c_str()) != 0) {
        std::cerr << "MySQL query failed: " << mysql_error(conn) << std::endl;
        std::cerr << "Failed SQL: " << sql.substr(0, 100) << "..." << std::endl;
        return false;
    }
    return true;
}

std::vector<std::vector<std::string>> MySQLConnection::query(const std::string& sql) {
    std::lock_guard<std::mutex> lock(dbMutex);
    if (!connected) connect();

    if (mysql_query(conn, sql.c_str()) != 0) {
        std::cerr << "Query failed: " << mysql_error(conn) << std::endl;
        return {};
    }

    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) return {};

    std::vector<std::vector<std::string>> rows;
    MYSQL_ROW row;
    unsigned int fields = mysql_num_fields(res);

    while ((row = mysql_fetch_row(res))) {
        std::vector<std::string> r;
        for (unsigned int i = 0; i < fields; ++i) {
            r.emplace_back(row[i] ? row[i] : "");
        }
        rows.push_back(std::move(r));
    }

    mysql_free_result(res);
    return rows;
}

std::string MySQLConnection::escape(const std::string& str) const {
    if (!conn || !connected) return str;
    char buffer[1024];
    mysql_real_escape_string(conn, buffer, str.c_str(), str.length());
    return std::string(buffer);
}


MYSQL* MySQLConnection::getConnection() const { return conn; }

MYSQL_RES* MySQLConnection::executeAndReturn(const std::string& sql) {
    execute(sql);
    return mysql_store_result(conn);
}

void MySQLConnection::freeResult(MYSQL_RES* res) {
    if (res) mysql_free_result(res);
}

std::size_t MySQLConnection::getNumRows(MYSQL_RES* res) {
    return res ? mysql_num_rows(res) : 0;
}

MYSQL_ROW MySQLConnection::getRow(MYSQL_RES* res) {
    return res ? mysql_fetch_row(res) : nullptr;
}