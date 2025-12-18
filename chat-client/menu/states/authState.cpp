// ALL WORKING THERE

#include "authState.hpp"
#include "../menuManager.hpp"
#include "mainState.hpp"

void AuthState::display(MenuManager& m) {
    m.clear();
    std::cout << "1. Login " << std::endl;
    std::cout << "2. Register" << std::endl;
    std::cout << "3. Exit " << std::endl;
}

void AuthState::handleInput(MenuManager& m, const std::string& in) {
    if (in == "1") {
        m.clear();
        std::string user = m.inputString("Username: ");
        std::string pass = m.passwordInput();
        UserInfo info;
        info.username = user;
        if (m.auth().login(info,pass)) {
            m.userSession().login(info);
            m.switchTo<MainState>();
            std::cout << "Login successful!\n";
        } else {
            std::cout << "Invalid credentials!\n";
            std::exit(0);
        }
    }
    else if (in == "2") {
        m.clear();
        UserInfo info;
        info.username = m.inputString("Username: ");
        info.firstName = m.inputString("First name: ");
        info.secondName = m.inputString("Last name: ");
        info.email = m.inputString("Email: ");
        std::string pass = m.passwordInput();

        if (m.auth().registerUser(info, pass)) {
            m.userSession().login(info);
            m.switchTo<MainState>();
            std::cout << "Registered & logged in!\n";
        } else {
            std::cout << "Username or email taken!\n";
        }
        m.pause();
    }
    else if (in == "3") exit(0);
}