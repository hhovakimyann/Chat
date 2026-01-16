#ifndef IAUTH_SERVICE_HPP
#define IAUTH_SERVICE_HPP

#include "../../shared/session/userSession.hpp"
#include <optional>
#include <string>

class IAuthService {
public:
  virtual ~IAuthService() = default;
  virtual std::optional<UserInfo> login(const std::string &username,
                                        const std::string &password) = 0;
  virtual std::optional<UserInfo> registerUser(const std::string &username,
                                               const std::string &firstName,
                                               const std::string &secondName,
                                               const std::string &email,
                                               const std::string &password) = 0;
};

#endif
