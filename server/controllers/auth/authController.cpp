#include "authController.hpp"
#include "../../../shared/session/userSession.hpp"
#include "notifier/notificationManager.hpp"
#include "utils/jwt/jwt.hpp"
#include <iostream>

AuthController::AuthController(std::unique_ptr<IAuthService> as) : authService(std::move(as)) {}

nlohmann::json AuthController::login(const nlohmann::json &req, int clientSocket) {
  try {
    std::cout << "Got request:" << req << std::endl;
    std::cout << "Trying to get username and password" << std::endl;
    auto username = req.value("username", "");
    std::cout << "Got username: " << username << std::endl;
    auto password = req.value("password", "");
    std::cout << "Got password: " << password << std::endl;
    if (username.empty() || password.empty()) {
      return {{"status", "error"}, {"message", "Missing Credentials"}};
    }
    std::cout << "Trying to login" << std::endl;
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
  std::cout << "Got request:" << req << std::endl;
  std::string username = req.value("username", "");
  std::cout << "Got username: " << std::endl;
  std::string firstName = req.value("first_name", "");
  std::cout << "Got first name: " << std::endl;
  std::string secondName = req.value("second_name", "");
  std::cout << "Got second name: " << std::endl;
  std::string email = req.value("email", "");
  std::cout << "Got email: " << email << std::endl;
  std::string password = req.value("password", "");
  std::cout << "Got password: " << std::endl;

  if (username.empty() || email.empty() || password.empty()) {
    return {{"status", "error"}, {"message", "All fields are required"}};
  }
  std::cout << "Trying to register" << std::endl;
  auto info = authService->registerUser(username, firstName, secondName, email, password);
  if (info.has_value()) {
    std::cout << "Registration successful: Trying to make Online in redis" << std::endl;
    RealTimeManager::getInstance().markOnline(username, clientSocket);
    std::cout << "Successfull Redis online Operation" << std::endl;
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
