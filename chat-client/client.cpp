#include "menu/menuManager.hpp"
#include "services/auth/authService.hpp"
#include "services/dm/dmService.hpp"
#include "services/group/groupService.hpp"
#include "../shared/session/userSession.hpp"
#include "network/clientNetwork.hpp"

int main() {
    auto network = std::make_unique<ClientNetwork>();
    network->connectServer();

    UserSession session;
    ClientAuthService   auth(network.get(), session);
    ClientDMService       dm(network.get(), session, &auth);
    ClientGroupService  group(network.get(), session, &auth);

    MenuManager app(auth, dm, group, session, network.get());
    app.run();

    return 0;
}