#ifndef MENU_MENARER_HPP
#define MENU_MENARER_HPP

#include "../../shared/session/userSession.hpp"
#include "../utils/inputCheckers.hpp"
#include "../interfaces/INetwork.hpp"
#include "../interfaces/IAuthService.hpp"
#include "../interfaces/IDMService.hpp"
#include "../interfaces/IGroupService.hpp"
#include "states/IMenuState.hpp"
#include "states/authState.hpp"
#include "states/dmState.hpp"
#include "states/groupState.hpp"
#include "states/mainState.hpp"
#include <memory>
#include <atomic>
#include <variant>

class MenuManager {
    private:
    IAuthService&   authService;
    IDMService&     dmService;
    IGroupService&  groupService;
    INetwork* networkClient;
    UserSession&    session;
    
    std::variant<AuthState, MainState, DMState, GroupState> currentState;
    
    bool inChatMode = false;
    std::string chatTarget;
    public:
    MenuManager(IAuthService&, IDMService&, IGroupService&, UserSession&, INetwork*); // ++
    void run(); // ++
    

    template<typename S, typename... Args>
    void switchTo(Args&&... args) {
        currentState.emplace<S>(std::forward<Args>(args)...);
    }
    
    void enterChat(const std::string& target);
    void exitChat();
    
    void clear() const; //++
    void pause() const; // ++
    std::string inputString(const std::string& prompt = ">") const; // ++
    int inputInt(const std::string& prompt) const; // ++
    std::string passwordInput() const;
    
    
    auto& auth() const { return authService; } // ++
    auto& dm() const { return dmService; } // ++
    auto& group() const { return groupService; } // ++
    auto& userSession() const { return session; } // ++
    auto& network() const {return networkClient;} // ++
    
    
    const std::string& target() const { return chatTarget; }
    void setTarget(const std::string& target) {chatTarget = target;}
    bool isChatting() const { return inChatMode; }
    
    void displayCurrent();
    void handleInputCurrent(const std::string& input);
};



#endif //CLINET_MENU_HPP