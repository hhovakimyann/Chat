#ifndef MIGRATION_HPP
#define MIGRATION_HPP

#include "../../database/IDBconnection.hpp"
#include <string>
#include <vector>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdexcept>

template <class ConcreteDB>
class Migration {
private:
    ConcreteDB *db;
    
    std::string extractVersion(const std::string& filename) {
        std::size_t pos = filename.find('_');
        if(pos == std::string::npos) {
            return "";
        }

        std::string version = filename.substr(0, pos);

        if(version.empty() || !std::all_of(version.begin(),version.end(),  [](unsigned char c){ return std::isdigit(c); })) {
            return "";
        }

        return version;
    }
    
    bool isMigrationApplied(const std::string& version) {
        std::string escaped = db->escapeString(version);
        std::string query = "SELECT 1 FROM migrations_file WHERE filename = '" + escaped + "'";

        auto* res = db->executeAndReturn(query);
        if (!res) {
            std::cerr << "Query failed for version " << version << std::endl;
            return false;
        }

        std::size_t rows = db->getNumRows(res);
        db->freeResult(res);
        return rows > 0;
    }
    
    bool recordMigration(const std::string& version) {
        std::string escapedString = db->escapeString(version);
        std::string query = "INSERT INTO migrations_file (filename) VALUES ('" + escapedString + "')";
        return db->execute(query);
    }
    
    bool removeMigrationRecord(const std::string& version) {
        std::string escapedString = db->escapeString(version);
        std::string query = "DELETE FROM migrations_file WHERE filename = '" + escapedString + "' ";
        return db->execute(query);
    }
    
    std::vector<std::filesystem::path> getMigrationFiles(const std::string& direction) {
        namespace fs = std::filesystem;
        std::vector<fs::path> files;
        fs::path dir = fs::path("migrations") / direction;

        if (!fs::exists(dir)) {
            std::cerr << "Migration directory not found: " << dir << std::endl;
            return files;
        }

        for (const auto& entry : fs::directory_iterator(dir)) {
            if (entry.path().extension() == ".sql") {
                files.push_back(entry.path());
            }
        }

        std::sort(files.begin(), files.end(),
            [this](const fs::path& a, const fs::path& b) {
                return extractVersion(a.filename().string()) <
                       extractVersion(b.filename().string());
            });

        return files;
    }
    
public:
    explicit Migration(ConcreteDB* database) : db(database) {
        if(!db->isConnected()) db->connect();
    }
    
    void runUp() { 
        std::cout << "Migration UP started " << std::endl;
        auto files = getMigrationFiles("up");

        db->execute("CREATE TABLE IF NOT EXISTS migrations_file (id INT AUTO_INCREMENT PRIMARY KEY, filename VARCHAR(255) UNIQUE NOT NULL, applied_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP)");
        std::cout << "Migration table created " << std::endl;

        for(const auto& file : files) {
            std::string filename = file.filename().string();
            std::string version = extractVersion(filename);
            std::cout << "Version extracted " << version << std::endl;

            if(version.empty()) {
                std::cerr << "Invalide File " << version << std::endl;
                continue;
            }

            if(isMigrationApplied(version)) {
                std::cout << "Migration Applied" << version << std::endl;
                continue;
            }

            std::ifstream sqlFile(file);
            if(!sqlFile) continue;
            
            std::stringstream buffer;
            buffer << sqlFile.rdbuf();
            std::string sql = buffer.str();
            std::cout << sql << std::endl;

            if (!db->execute(sql)) {
                std::cerr << "Failed: " << filename << std::endl;
                throw std::runtime_error("Migration failed");
            }

            if (!recordMigration(version)) {
                throw std::runtime_error("Failed to record");
            }
            std::cout << "Applied: " << filename << std::endl;
        }
    }
    
    void runDown() {
        auto files = getMigrationFiles("down");
        std::reverse(files.begin(), files.end());  
        for (const auto& file : files) {
            std::string filename = file.filename().string();
            std::string version = extractVersion(filename);
            if (version.empty()) continue;

            if (!isMigrationApplied(version)) {
                std::cout << "Skipping (not applied): " << filename << std::endl;
                continue;
            }

            std::cout << "Rolling back: " << filename << std::endl;

            std::ifstream sqlFile(file);
            if (!sqlFile) continue;
            std::stringstream buffer;
            buffer << sqlFile.rdbuf();
            std::string sql = buffer.str();

            db->execute("START TRANSACTION");
            db->execute("SET FOREIGN_KEY_CHECKS=0");

            if (!db->execute(sql)) {
                std::cerr << "Rollback failed: " << filename << std::endl;
                db->execute("ROLLBACK");
                db->execute("SET FOREIGN_KEY_CHECKS=1");
                continue;
            }

            removeMigrationRecord(version);
            db->execute("COMMIT");
            db->execute("SET FOREIGN_KEY_CHECKS=1");
            std::cout << "Rolled back: " << filename << std::endl;
        }
    }
};

#endif // MIGRATION_HPP