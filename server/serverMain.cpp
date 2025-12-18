#include "server.hpp"
#include "../database/MySQLconnection.hpp"
#include "../migrations/migration.hpp"
#include "../config/env_loader.hpp"
#include "controllers/auth/authController.hpp"
#include "controllers/dm/dmController.hpp"
#include "controllers/group/groupController.hpp"
#include "service/auth/authService.hpp"
#include "service/dm/dmService.hpp"
#include "service/group/groupService.hpp"
#include "network/requestRouter.hpp"
#include <iostream>
#include <csignal>
#include <memory>

Server* globalServer = nullptr;

void signalHandler(int signal) {
    std::cout << "\nReceived signal " << signal << ". Shutting down gracefully...\n";
    if (globalServer) {
        globalServer->stop();
    }
    std::exit(0);
}

int main(int argc, char* argv[]) {
    EnvLoader::load(".env");

    if (argc >= 3 && std::string(argv[1]) == "migrate") {
        std::string direction = argv[2];

        if (direction != "up" && direction != "down") {
            std::cerr << "Error: Migration direction must be 'up' or 'down'\n";
            std::cerr << "Usage: " << argv[0] << " migrate up|down\n";
            return 1;
        }

        auto conn = std::make_unique<MySQLConnection>();
        std::cout << "MYSQL OBJECT CREATED" << std::endl;

        try {
            conn->connect();
            std::cout << "Connected to database for migration.\n";
        } catch (const std::exception& e) {
            std::cerr << "Database connection failed: " << e.what() << "\n";
            return 1;
        }
        
        std::cout << "CONNECTED" << std::endl;

        conn->execute(R"(
            CREATE TABLE IF NOT EXISTS migrations_file (
                id INT AUTO_INCREMENT PRIMARY KEY,
                filename VARCHAR(255) NOT NULL UNIQUE,
                migrated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
            )
        )");

        Migration<MySQLConnection> migrator(conn.get());
        if (direction == "up") {
            migrator.runUp();
            std::cout << "Migrations applied successfully.\n";
        } else {
            migrator.runDown();
            std::cout << "Migrations rolled back.\n";
        }

        return 0;
    }

    if (argc > 1) {
        std::cout << "Usage:\n";
        std::cout << "  " << argv[0] << "              - Start server\n";
        std::cout << "  " << argv[0] << " migrate up   - Apply migrations\n";
        std::cout << "  " << argv[0] << " migrate down - Rollback migrations\n";
        return 1;
    }

    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    std::cout << "Starting SecureChat server on port 8080...\n";

    auto db = std::make_unique<MySQLConnection>();
    db->connect();

    auto authService = std::make_unique<ServerAuthService>(*db);
    auto dmService = std::make_unique<DMService>(*db);
    auto groupService = std::make_unique<GroupService>(*db);

  
    AuthController authCtrl(std::move(authService));
    DMController dmCtrl(std::move(dmService));
    GroupController groupCtrl(std::move(groupService));

    RequestRouter router(authCtrl,dmCtrl, groupCtrl);

    Server server(router);
    globalServer = &server;

    std::cout << "Starting SecureChat server on port 8080...\n";

    if (!server.start(8080)) {
        std::cerr << "Failed to start server!\n";
        return 1;
    }

    while (globalServer) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}