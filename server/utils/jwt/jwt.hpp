#pragma once
#include "../../../config/env_loader.hpp"
#include <string>
#include <optional>
#include <jwt-cpp/jwt.h>
#include <stdexcept>

class JWT {
private:
    static const std::string& getSecret() {
        EnvLoader::load(".env");
        static std::string secret = EnvLoader::get("JWT_SECRET");
        return secret;
    }

public:
    static std::string encode(const std::string& username, int userId) {
        return jwt::create()
            .set_issuer("securechat")
            .set_type("JWS")
            .set_payload_claim("username", jwt::claim(username))
            .set_payload_claim("user_id", jwt::claim(std::to_string(userId)))
            .set_issued_at(std::chrono::system_clock::now())
            .set_expires_at(std::chrono::system_clock::now() + std::chrono::hours{24})
            .sign(jwt::algorithm::hs256{getSecret()});
    }

    static std::optional<jwt::decoded_jwt<jwt::traits::kazuho_picojson>> decode(const std::string& token) {
        try {
            auto decoded = jwt::decode(token);

            auto verifier = jwt::verify()
                .allow_algorithm(jwt::algorithm::hs256{getSecret()})
                .with_issuer("securechat");

            verifier.verify(decoded);
            return decoded;
        } catch (...) {
            return std::nullopt;
        }
    }

    static std::optional<std::string> getUsername(const std::string& token) {
        auto decoded = decode(token);
        if (!decoded) return std::nullopt;

        try {
            return decoded->get_payload_claim("username").as_string();
        } catch (...) {
            return std::nullopt;
        }
    }

    static std::optional<int> getUserId(const std::string& token) {
        auto decoded = decode(token);
        if (!decoded) return std::nullopt;

        try {
            return std::stoi(decoded->get_payload_claim("user_id").as_string());
        } catch (...) {
            return std::nullopt;
        }
    }
};
