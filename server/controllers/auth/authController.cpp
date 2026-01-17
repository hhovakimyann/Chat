#include "authController.hpp"
#include "../../../shared/session/userSession.hpp"
#include "notifier/notificationManager.hpp"
#include "utils/jwt/jwt.hpp"
#include <iostream>

AuthController::AuthController(std::unique_ptr<IAuthService> as) : authService(std::move(as)) {}

nlohmann::json AuthController::login(const nlohmann::json &req, int clientSocket) {
  try {
    auto username = req.value("username", "");
    auto password = req.value("password", "");
    if (username.empty() || password.empty()) {
      return {{"status", "error"}, {"message", "Missing Credentials"}};
    }

    auto info = authService->login(username, password);
    if (info.has_value()) {
        RealTimeManager::getInstance().markOnline(username, clientSocket);
        return {
          {"status", "success"},
          {"accessToken", info->accessToken},
          {"refreshToken", info->refreshToken},
          {"user",
          {
            {"username", info->username},
            {"first_name", info->firstName},
            {"second_name", info->secondName},
            {"email", info->email}
          }
        }
      };
    }
    return {{"status", "error"}, {"message", "Wrong Username or Password"}};
  } catch (const nlohmann::json::exception &e) {
    nlohmann::json response;
    response["status"] = "error";
    response["message"] = "Invalid request: " + std::string(e.what());
    return response;
  } catch (const std::exception &e) {
    nlohmann::json response;
    response["status"] = "error";
    response["message"] = "Invalid request: " + std::string(e.what());
    return response;
  }
}

nlohmann::json AuthController::registerUser(const nlohmann::json &req,int clientSocket) {
  std::string username = req.value("username", "");
  std::string firstName = req.value("first_name", "");
  std::string secondName = req.value("second_name", "");
  std::string email = req.value("email", "");
  std::string password = req.value("password", "");

  if (username.empty() || email.empty() || password.empty()) {
    return {{"status", "error"}, {"message", "All fields are required"}};
  }
  auto info = authService->registerUser(username, firstName, secondName, email, password);
  if (info.has_value()) {
    RealTimeManager::getInstance().markOnline(username, clientSocket);
    return {
      {"status", "success"},
      {"message", "Registration successful"},
      {"accessToken", info->accessToken},
      {"refreshToken", info->refreshToken},
      {"user",
       {  
        {"username", info->username},
        {"first_name", info->firstName},
        {"second_name", info->secondName},
        {"email", info->email}
       }
      }
    };
  }

  return {{"status", "error"}, {"message", "Username or email already taken"}};
}

nlohmann::json AuthController::refresh(const nlohmann::json &req) {
    try {
        std::string refreshToken = req.value("refreshToken", "");
        if (refreshToken.empty()) {
             return {{"status", "error"}, {"message", "Missing refresh token"}};
        }

        auto decoded = JWT::decode(refreshToken);
        if (!decoded) {
            return {{"status", "error"}, {"message", "Invalid refresh token"}};
        }

        std::string username = decoded->get_payload_claim("username").as_string();
        int userId = std::stoi(decoded->get_payload_claim("user_id").as_string());
        
        auto tokens = JWT::generateTokens(username, userId);
        
        return {
            {"status", "success"},
            {"accessToken", tokens.accessToken},
            {"refreshToken", tokens.refreshToken} 
        };

    } catch (...) {
        return {{"status", "error"}, {"message", "Internal server error during refresh"}};
    }
}
