#include "dmState.hpp"
#include "../menuManager.hpp"
#include "mainState.hpp"
#include "../../shared/session/userSession.hpp"
#include "../../interfaces/IDMService.hpp"
#include "../../../nlohmann/json.hpp"
#include <iostream>
#include <vector>

DMState::~DMState() {
    isChatActive = false;
    if (listenerThread && listenerThread->joinable()) {
        listenerThread->join();
    }
}

bool DMState::checkUserExists(MenuManager& manager, const std::string& username) {
    return manager.dm().checkUserExists(username);
}

void DMState::startListening(MenuManager& manager) {
    while (isChatActive) {
        std::string rawResponse;

        if (!manager.network() || !manager.network()->isConnected()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }
        
        try{
            rawResponse = manager.network()->receiveData();

            std::cout << "Raw Responce " << rawResponse << std::endl;
            if (rawResponse.empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                continue;
            }

            std::cout << "[DEBUG] Received: " << rawResponse.substr(0, 100) << std::endl;            
            auto json = nlohmann::json::parse(rawResponse);

            if (json.contains("type") && json["type"] == "new_message") {
                std::string sender = json["from"];
                if (sender == currentTargetUser) {
                    std::lock_guard<std::mutex> lock(consoleMtx);
                    
                    std::cout << "\r\033[K"; 
                    std::cout << "[" << sender << "]: " << json["content"] << "\n";
                    std::cout << "> " << std::flush;
                } else {
                    std::lock_guard<std::mutex> lock(consoleMtx);
                    std::cout << "\r\033[K[System] New message from " << sender << "\n> " << std::flush;
                }
            }
            else if (json.contains("status") && json["status"] == "success") {
                if (json["status"] == "error") {
                    std::lock_guard<std::mutex> lock(consoleMtx);
                    std::cout << "\r\033[K[Error] " << json["message"] << "\n> " << std::flush;
                }
            }
        }catch(const std::exception& e) {
            std::cout << "Catch in Listener Thread " << e.what() << std::endl;
        }
        
    }
}

void DMState::enterChatMode(MenuManager& manager, const std::string& target) {
    currentTargetUser = target;
    isChatActive = true;

    std::cout << "Chat with @" << target << " (type /back to leave)\n";
    std::cout << std::string(50, '-') << "\n";
    
    auto conversations = manager.dm().fetchMessages(target, 50);
    UserSession us;
    std::cout << us.getUsername() << std::endl;
    for (const auto& dm : conversations) {
        std::string prefix = (dm.sender == us.getUsername() ) ? "[You]" : "[" + dm.sender + "]";
        std::cout << prefix << ": " << dm.content << "\t\t " << dm.timestamp << std::endl;
    }
    std::cout << std::string(50, '-') << "\n";
    std::cout << "> " << std::flush;


    listenerThread = std::thread(&DMState::startListening, this, std::ref(manager));

    std::string input;
    while(isChatActive) {
       std::cin >> input;
        if(input == "/back" || input == "/exit") {
            std::cout << "EXITINGGGGGGGGG" << std::endl;
            isChatActive = false;
            break;
        }

        if(input.empty()) {
            std::cout << "> ";
            continue;
        }

        manager.dm().sendMessage(target,input);
        std::cout << "\033[A\r\033[K";
        std::cout << "[You] " << input << std::endl;
    }

    if(listenerThread->joinable()) {
        listenerThread->detach();
    }

    listenerThread.reset();
    manager.exitChat();
}

void DMState::display(MenuManager& manager) {
    std::cout << "Direct Messages\n";
    std::cout << "Existing Conversation With " << std::endl;
    auto partners = manager.dm().getAllConversations();
    for(const auto& partner : partners) {
        std::cout << ". " << partner.otherUser << "\t";
    }
    std::cout << std::endl;
    std::cout << "=================" << std::endl;
    std::cout << "1. Start new DM\n";
    std::cout << "2. Open existing conversation\n";
    std::cout << "3. Back to main menu\n";
}

void DMState::handleInput(MenuManager& manager, const std::string& input) {
    if (input == "1") {
        std::string username = manager.inputString("Enter username to message: @");
        if (!username.empty() && checkUserExists(manager, username)) {
            manager.enterChat(username);
            manager.setTarget(username);
            enterChatMode(manager,username);
            std::cout << "Exiting from chat" << std::endl;
        } else {
            std::cout << "User not found!\n";
            manager.pause();
        }
    } else if (input == "2") {
        auto convos = manager.dm().getAllConversations();
        if (convos.empty()) {
            std::cout << "No conversations to open.\n";
            manager.pause();
            return;
        }

        std::string username = manager.inputString("Enter username to open: @");
        if (username.empty()) return;

        bool exists = false;
        for (const auto& c : convos) {
            if (c.otherUser == username) {
                exists = true;
                break;
            }
        }

        if (exists) {
            manager.enterChat(username);
            enterChatMode(manager, username);
        } else {
            std::cout << "No conversation with @" << username << "\n";
            manager.pause();
        }
    } else if (input == "3") {
        std::cout << "Changing state" << std::endl;
        manager.switchTo<MainState>();
        std::cout << "State changed" << std::endl;
    } else {
        std::cout << "Invalid option.\n";
        manager.pause();
    }
    
}