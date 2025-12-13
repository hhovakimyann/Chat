#ifndef IDATABASECONNECTION_HPP
#define IDATABASECONNECTION_HPP

#include <string>
template <class DataBaseTypeRes, class DataBaseType, class DataBaseRow>
class IDatabaseConnection {
public:
    virtual ~IDatabaseConnection() = default;

    virtual void connect() = 0;
    virtual void close() = 0;
    virtual bool isConnected() const = 0;

    virtual DataBaseType* getConnection() const = 0;
    virtual bool execute(const std::string& sql) = 0;
    virtual DataBaseTypeRes* executeAndReturn(const std::string& sql) = 0;

    virtual void freeResult(DataBaseTypeRes* res) = 0;
    virtual std::string escapeString(const std::string& str) const = 0;
    virtual std::size_t getNumRows(DataBaseTypeRes* res) = 0;
    virtual DataBaseRow getRow(DataBaseTypeRes* res) = 0;
};

#endif // IDATABASECONNECTION_HPP
