#include "env_loader.hpp"
#include <fstream>
#include <sstream>

std::unordered_map<std::string, std::string> EnvLoader::vars;

void EnvLoader::load(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = line.substr(0, pos);
        std::string val = line.substr(pos + 1);

        vars[key] = val;
    }
}

std::string EnvLoader::get(const std::string& key) {
    return vars[key];
}
