#ifndef IGROUP_SERVICE_HPP
#define IGROUP_SERVICE_HPP

#include "../../shared/messages/messages.hpp"
#include "IDMService.hpp"
#include <vector>

class IGroupService {
public:
    virtual ~IGroupService() = default;

    virtual std::vector<std::string> listGroups() = 0;
    virtual bool createGroup(const std::string& name) = 0;
    virtual bool joinGroup(const std::string& name) = 0;
    virtual bool leaveGroup(const std::string& name) = 0;

    virtual bool sendMessage( const std::string& groupName, const std::string& content) = 0;
    virtual std::vector<Message> fetchMessages(const std::string& groupName, int limit) = 0;
};

#endif
