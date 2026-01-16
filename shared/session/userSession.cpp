#include "userSession.hpp"


void UserSession::login(const UserInfo& info) {
    user = info;
}
void UserSession::logout() {
    user.reset();
}
bool UserSession::isLoggedIn() const {
    return user.has_value();
}

const std::string& UserSession::getAccessToken() const {
    static const std::string empty;
    return user ? user->accessToken : empty;
}

const UserInfo& UserSession::getUser() const {
    static const UserInfo emptyUser;
    return user ? *user : emptyUser;
}

const std::string& UserSession::getUsername() const {
    static const std::string empty;
    return user ? user->username : empty;
}