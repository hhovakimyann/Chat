#include "authService.hpp"
#include "../../utils/jwt/jwt.hpp"

ServerAuthService::ServerAuthService(IDatabase& connection) : db(connection) {
    if (!db.isConnected()) {
        db.connect();
    }
}

std::string ServerAuthService::generateJWT(int userId, const std::string& username) {
    return JWT::encode(username,userId);
}

bool ServerAuthService::usernameExists(const std::string& username) {
    std::string escaped = db.escape(username);
    std::string query = "SELECT 1 FROM users WHERE username = '" + escaped + "' LIMIT 1";
    auto rows = db.query(query);

    return !rows.empty();
}

bool ServerAuthService::emailExists(const std::string& email) {
    std::string escaped = db.escape(email);
    std::string query = "SELECT 1 FROM users WHERE email = '" + escaped + "' LIMIT 1";
    auto rows = db.query(query);

    return !rows.empty();
}
std::optional <UserInfo> ServerAuthService::login(const std::string& username,const std::string& password) {
    if(!db.isConnected()) db.connect();
    std::string escaped = db.escape(username);
    std::string query = 
        "SELECT id, password_hash, first_name, second_name, email FROM users "
        "WHERE username = '" + escaped + "' LIMIT 1";

    auto rows = db.query(query);
    if (rows.empty()) {
        return std::nullopt;
    }
    const auto& row = rows[0];
    int userId = std::stoi(row[0]);
    std::string storedHash = row[1];

    if (!Password::verifyPassword(password, storedHash)) {
        return std::nullopt;
    }
    std::string firstName = row[2];
    std::string secondName = row[3];
    std::string email = row[4];


    UserInfo info;
    info.username = username;
    info.firstName = firstName;
    info.secondName = secondName;
    info.email = email;
    info.jwt = generateJWT(userId, username);

    return info;
}


std::optional<UserInfo> ServerAuthService::registerUser(const std::string& username, const std::string& firstName, const std::string& secondName, const std::string& email,const std::string& password) {
    if (usernameExists(username)) return std::nullopt;
    if (emailExists(email)) return std::nullopt;
    if (password.length() < 6) return std::nullopt;

    std::string hash = Password::hashPassword(password);
    if (hash.empty()) return std::nullopt;;

    std::string query = "INSERT INTO users (username, first_name, second_name, email, password_hash) VALUES ("
        "'" + db.escape(username) + "', "
        "'" + db.escape(firstName) + "', "
        "'" + db.escape(secondName) + "', "
        "'" + db.escape(email) + "', "
        "'" + hash + "')";

    if (!db.execute(query)) {
        return std::nullopt;;
    }

    auto rows = db.query("SELECT LAST_INSERT_ID()");
    if (rows.empty()) {
        return std::nullopt;
    }
    int userId = std::stoi(rows[0][0]);
    UserInfo info;
    info.username = username;
    info.firstName = firstName;
    info.secondName = secondName;
    info.email = email;
    info.jwt = generateJWT(userId, info.username);

    return info;
}