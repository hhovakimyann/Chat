#pragma once
#include <string>
#include <vector>
#include <cstdint>

class IDatabase {
public:
    virtual ~IDatabase() = default;

    virtual void connect() = 0;
    virtual void close() = 0;
    virtual bool isConnected() const = 0;

    virtual bool execute(const std::string& sql) = 0;
    virtual std::vector<std::vector<std::string>> query(const std::string& sql) = 0;

    virtual std::string escape(const std::string& str) const = 0;
};