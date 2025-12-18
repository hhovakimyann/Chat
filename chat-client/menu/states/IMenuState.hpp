#ifndef IMENU_STATE_HPP
#define IMENU_STATE_HPP
#include <string>

class MenuManager;

class MenuState {
public:
    virtual ~MenuState() = default;
    virtual void display(MenuManager& manager) = 0;
    virtual void handleInput(MenuManager& manager, const std::string& input) = 0;
};

#endif