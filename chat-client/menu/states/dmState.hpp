#ifndef DM_STATE_HPP
#define DM_STATE_HPP

#include "IMenuState.hpp"
#include "../../../shared/messages/messages.hpp"
#include <vector>
#include <thread>
#include <optional>

class DMState : public MenuState {
private:
    std::atomic<bool> isChatActive{false};
    std::optional<std::thread> listenerThread;
    std::mutex consoleMtx;

    std::vector<Message> currentMessages;
    std::string currentTargetUser;

    bool checkUserExists(MenuManager& manager, const std::string& username);
    void startListening(MenuManager& manager);
public:
    void display(MenuManager& manager) override;
    void handleInput(MenuManager& manager, const std::string& input) override;
    void enterChatMode(MenuManager& manager, const std::string& target);

    virtual ~DMState();
};

#endif