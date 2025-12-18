#ifndef IAUTH_SERVICE_HPP
#define IAUTH_SERVICE_HPP

#include <string>
#include "../shared/session/userSession.hpp"

class IAuthService {
public:
    virtual ~IAuthService() = default;
    virtual bool login(UserInfo& info, const std::string& password) = 0;
    virtual bool registerUser(UserInfo& info, const std::string& password) = 0;

};

#endif
