# Compiler settings
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
LDFLAGS = 

# Directories
BUILD_DIR = build
SERVER_EXEC = server-chat
CLIENT_EXEC = client-chat

# External libraries - macOS paths
JWT_INCLUDE = -I/usr/local/include
JSON_INCLUDE = -I/usr/local/include -I/opt/homebrew/include -I./libs/nlohmann
SODIUM_LIBS = -L/opt/homebrew/lib -lsodium
MYSQL_LIBS = -L/opt/homebrew/lib -lmysqlclient
SSL_LIBS = -L/opt/homebrew/lib -lssl -lcrypto
LIBRARY_PATHS = -L/usr/local/lib -L/opt/homebrew/lib

# MySQL include path
MYSQL_INCLUDE = -I/opt/homebrew/include/mysql -I/usr/local/include/mysql -I/usr/local/mysql/include

# Server source files
SERVER_SRCS = \
	server/controllers/auth/authController.cpp \
	server/controllers/dm/dmController.cpp \
	server/controllers/group/groupController.cpp \
	server/network/requestRouter.cpp \
	server/password/password.cpp \
	database/MySQLconnection.cpp \
	server/service/auth/authService.cpp \
	server/service/dm/dmService.cpp \
	server/service/group/groupService.cpp \
	server/utils/string/stringUtils.cpp \
	server/server.cpp \
	server/serverMain.cpp \
	config/env_loader.cpp \
	server/notifier/notificationManager.cpp 



CLIENT_SRCS = \
	chat-client/menu/states/authState.cpp \
	chat-client/menu/states/dmState.cpp \
	chat-client/menu/states/groupState.cpp \
	chat-client/menu/states/mainState.cpp \
	chat-client/menu/menuManager.cpp \
	chat-client/services/auth/authService.cpp \
	chat-client/services/dm/dmService.cpp \
	chat-client/services/group/groupService.cpp \
	chat-client/services/network/clientNetwork.cpp \
	chat-client/utils/inputCheckers.cpp \
	chat-client/client.cpp \
	config/env_loader.cpp \
    shared/session/userSession.cpp 
# Generate object files
SERVER_OBJS = $(SERVER_SRCS:%.cpp=$(BUILD_DIR)/server/%.o)
CLIENT_OBJS = $(CLIENT_SRCS:%.cpp=$(BUILD_DIR)/client/%.o)

# Include paths for server
SERVER_INCLUDES = -I. \
	-I./server \
	-I./server/controllers \
	-I./server/controllers/auth \
	-I./server/controllers/dm \
	-I./server/controllers/group \
	-I./server/interfaces \
	-I./server/network \
	-I./server/password \
	-I./database \
	-I./migrations \
	-I./server/service \
	-I./server/service/auth \
	-I./server/service/dm \
	-I./server/service/group \
	-I./server/utils \
	-I./server/utils/jwt \
	-I./server/utils/string \
	-I./config \
	$(JWT_INCLUDE) \
	$(JSON_INCLUDE) \
	$(MYSQL_INCLUDE)


CLIENT_INCLUDES = -I. \
	-I./chat-client \
	-I./chat-client/menu \
	-I./chat-client/menu/states \
	-I./chat-client/services \
	-I./chat-client/services/auth \
	-I./chat-client/services/dm \
	-I./chat-client/services/group \
	-I./chat-client/services/network \
	-I./chat-client/utils \
	-I./config \
	$(JSON_INCLUDE)

# Library paths
SERVER_LIBS = $(LIBRARY_PATHS) $(SODIUM_LIBS) $(MYSQL_LIBS) $(SSL_LIBS) -lpthread
CLIENT_LIBS = $(LIBRARY_PATHS) $(SODIUM_LIBS) $(SSL_LIBS)

# Default target
all: server client

# Server target
server: $(SERVER_EXEC)

# Client target
client: $(CLIENT_EXEC)

# Link server executable
$(SERVER_EXEC): $(SERVER_OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(SERVER_LIBS)
	@echo "Server built successfully: $(SERVER_EXEC)"

# Link client executable
$(CLIENT_EXEC): $(CLIENT_OBJS)
	@mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(CLIENT_LIBS)
	@echo "Client built successfully: $(CLIENT_EXEC)"

# Compile server source files
$(BUILD_DIR)/server/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(SERVER_INCLUDES) -c $< -o $@

# Compile client source files
$(BUILD_DIR)/client/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CLIENT_INCLUDES) -c $< -o $@

# Check if UserSession files exist
check-usersession:
	@echo "Checking UserSession files..."
	@test -f chat-client/menu/UserSession.cpp && echo "✓ UserSession.cpp exists" || echo "✗ UserSession.cpp missing"
	@test -f chat-client/menu/UserSession.hpp && echo "✓ UserSession.hpp exists" || echo "✗ UserSession.hpp missing"

# Clean build files
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(SERVER_EXEC) $(CLIENT_EXEC)
	@echo "Cleaned build files and executables"

# Rebuild everything
rebuild: clean all

# Show which source files are being compiled
show-sources:
	@echo "Client source files:"
	@for file in $(CLIENT_SRCS); do echo "  $$file"; done
	@echo ""
	@echo "Server source files:"
	@for file in $(SERVER_SRCS); do echo "  $$file"; done

.PHONY: all server client clean rebuild check-usersession show-sources