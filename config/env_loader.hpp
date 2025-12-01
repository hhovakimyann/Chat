#pragma once
#include <string>
#include <unordered_map>

class EnvLoader {
public:
    static void load(const std::string& filename);
    static std::string get(const std::string& key);

private:
    static std::unordered_map<std::string, std::string> vars;
};
