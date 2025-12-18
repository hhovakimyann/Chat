#pragma once
#include "IMenuState.hpp"

class MainState : public MenuState {
public:
    ~MainState() = default;
    void display(MenuManager& manager) override;
    void handleInput(MenuManager& manager, const std::string& input) override;
};