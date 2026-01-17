#ifndef USER_SESSION_HPP
#define USER_SESSION_HPP

#include <string>
#include <optional>

struct UserInfo {
    std::string username;
    std::string firstName; 
    std::string secondName;
    std::string email;
    std::string accessToken;
    std::string refreshToken;
};

class UserSession {
private:
    std::optional<UserInfo> user;

public:
    void login(const UserInfo& info);
    void logout();           
    bool isLoggedIn() const;

    const std::string& getAccessToken() const;
    const UserInfo& getUser() const; 
    const std::string& getUsername() const;
};

#endif