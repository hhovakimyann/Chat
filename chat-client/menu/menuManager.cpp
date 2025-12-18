#include "menuManager.hpp"
#include "states/mainState.hpp"
#include "states/authState.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <limits>
#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory>

MenuManager::MenuManager(IAuthService& a, IDMService& d, IGroupService& g, UserSession& s, INetwork* n) 
    : authService(a), dmService(d), groupService(g), networkClient(n), session(s) {}


void MenuManager::run() {
    switchTo<AuthState>();
    while (true) {
        displayCurrent();
        std::string choice = inputString();
        handleInputCurrent(choice);
    }
}

void MenuManager::displayCurrent() {
    std::visit([this](auto& state) {
        state.display(*this);
    }, currentState);
}

void MenuManager::handleInputCurrent(const std::string& input) {
    std::visit([this, &input](auto& state) {
        state.handleInput(*this, input);
    }, currentState);
}


void MenuManager::enterChat(const std::string& target) {
    chatTarget = target;
    inChatMode = true;
}

void MenuManager::exitChat() {
    inChatMode = false;
    chatTarget.clear();
    displayCurrent();
}

void MenuManager::clear() const { system("clear"); }

void MenuManager::pause() const {
    std::cout << "\nPress Enter to continue...";
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    std::cin.get();
}

std::string MenuManager::inputString(const std::string& prompt) const {
    std::string line;
    std::cout << prompt;
    std::getline(std::cin >> std::ws, line);
    return line;
}

int MenuManager::inputInt(const std::string& prompt) const {
    int x;
    std::cout << prompt;
    while (!(std::cin >> x)) {
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        std::cout << "Invalid number. Try again: ";
    }
    std::cin.ignore(10000, '\n');
    return x;
}

std::string MenuManager::passwordInput() const {
    std::cout << "Enter the password: ";
    termios oldt, newt;
    tcgetattr(0, &oldt);
    newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(0, TCSANOW, &newt);

    std::string input;
    std::getline(std::cin, input);

    tcsetattr(0, TCSANOW, &oldt);
    std::cout << std::endl;

    return input;
}