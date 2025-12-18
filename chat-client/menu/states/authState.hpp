// ALL WORLING THERE
#ifndef MAIN_STATE_HPP
#define MAIN_STATE_HPP

#include "IMenuState.hpp"

class AuthState : public MenuState {
public:
    ~AuthState() = default;
    void display(MenuManager& manager) override;
    void handleInput(MenuManager& manager, const std::string& input) override;
};

#endif