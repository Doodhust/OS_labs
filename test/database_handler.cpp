#include <sqlite3.h>
#include <iostream>
#include <ctime>

class DatabaseHandler {
public:
    DatabaseHandler(const std::string& dbPath) {
        if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
            std::cerr << "Failed to open database: " << sqlite3_errmsg(db) << std::endl;
        } else {
            createTable();
        }
    }

    ~DatabaseHandler() {
        sqlite3_close(db);
    }

    void logTemperature(double temperature) {
        std::time_t now = std::time(nullptr);
        std::string timestamp = std::ctime(&now);
        timestamp.pop_back();

        std::string query = "INSERT INTO temperatures (timestamp, temperature) VALUES ('" + timestamp + "', " + std::to_string(temperature) + ");";
        executeQuery(query);
    }

private:
    sqlite3* db;

    void createTable() {
        std::string query = "CREATE TABLE IF NOT EXISTS temperatures (id INTEGER PRIMARY KEY, timestamp TEXT, temperature REAL);";
        executeQuery(query);
    }

    void executeQuery(const std::string& query) {
        char* errMsg;
        if (sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
    }
};
