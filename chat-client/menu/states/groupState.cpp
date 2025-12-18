#include "groupState.hpp"
#include "../menuManager.hpp"
#include "mainState.hpp"
#include <iostream>

void GroupState::display(MenuManager& manager) {
    manager.clear();

    if (manager.isChatting()) {
        std::string groupName = manager.target();
        std::cout << "Group: #" << groupName << "  (type /back to leave)\n";
        std::cout << std::string(60, '-') << "\n";

        auto messages = manager.group().fetchMessages(groupName, 50);
        for (const auto& msg : messages) {
            std::cout << msg.sender << " [" << msg.timestamp.substr(11,5) << "]: " << msg.content << "\n";
        }
        std::cout << "\n> ";
    } else {
        std::cout << "Groups\n";
        std::cout << std::string(40, '=') << "\n";

        auto groups = manager.group().listGroups();
        if (groups.empty()) {
            std::cout << "You are not in any group.\n";
        } else {
            std::cout << "Your groups:\n";
            for (const auto& g : groups) {
                std::cout << " • #" << g << "\n";
            }
        }

        std::cout << "\n1. Create new group\n";
        std::cout << "2. Join group\n";
        std::cout << "3. Open group chat\n";
        std::cout << "4. Back to main\n\n";
        std::cout << "> ";
    }
}

void GroupState::handleInput(MenuManager& manager, const std::string& input) {
    if (manager.isChatting()) {
        if (input == "/back" || input == "/exit") {
            manager.exitChat();
            return;
        }
        if (!input.empty()) {
            manager.group().sendMessage(manager.target(), input);
            display(manager);
        }
        return;
    }

    if (input == "1") {
        std::string name = manager.inputString("Group name: #");
        if (name.empty()) return;
        if (manager.group().createGroup(name)) {
            std::cout << "Group #" << name << " created!\n";
        } else {
            std::cout << "Failed or group already exists.\n";
        }
        manager.pause();
    }
    else if (input == "2") {
        std::string name = manager.inputString("Enter group name to join: #");
        if (name.empty()) return;
        if (manager.group().joinGroup(name)) {
            std::cout << "Joined #" << name << "\n";
        } else {
            std::cout << "Group not found or already member.\n";
        }
        manager.pause();
    }
    else if (input == "3") {
        auto groups = manager.group().listGroups();
        if (groups.empty()) {
            std::cout << "No groups to open.\n";
            manager.pause();
            return;
        }
        std::string name = manager.inputString("Enter group name to open: #");
        if (name.empty()) return;

        bool found = false;
        for (const auto& g : groups) {
            if (g == name) { found = true; break; }
        }

        if (found) {
            manager.enterChat(name);
        } else {
            std::cout << "You're not in #" << name << "\n";
            manager.pause();
        }
    }
    else if (input == "4") {
        manager.switchTo<MainState>();
    }
    else {
        std::cout << "Invalid option.\n";
        manager.pause();
    }
}