#include "mainState.hpp"
#include "../menuManager.hpp"
#include "dmState.hpp"
#include "groupState.hpp"
#include "authState.hpp"

void MainState::display(MenuManager& manager) {
    manager.clear();
    std::cout << "Main Menu" << std::endl;
    std::cout << "1. Direct Messages" << std::endl;
    std::cout << "2. Groups" << std::endl;
    std::cout << "3. Logout" << std::endl;
}

void MainState::handleInput(MenuManager& manager, const std::string& input) {
    if (input == "1") {
        manager.switchTo<DMState>();
    } else if (input == "2") {
        manager.switchTo<GroupState>();
    } else if (input == "3") {
        manager.userSession().logout();
        manager.switchTo<AuthState>();
    } else {
        std::cout << "Invalid option!" << std::endl;
        manager.pause();
    }
}