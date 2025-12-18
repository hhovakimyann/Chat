#ifndef GROUP_STATE_HPP
#define GROUP_STATE_HPP

#include "IMenuState.hpp"

class GroupState : public MenuState {
public:
    void display(MenuManager& manager) override;
    void handleInput(MenuManager& manager, const std::string& input) override;
};

#endif