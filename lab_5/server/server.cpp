#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <ctime>
#include <sqlite3.h>
#include <httplib.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// ==================== SerialReader ====================
class SerialReader {
public:
    inline SerialReader(const std::string& port) : port(port) {}

    inline std::string read() {
        std::ifstream serial(port);
        std::string data;
        if (serial.is_open()) {
            std::getline(serial, data);
        } else {
            std::cerr << "Failed to open serial port: " << port << std::endl;
        }
        return data;
    }

private:
    std::string port;
};

// ==================== DatabaseHandler ====================
class DatabaseHandler {
public:
    inline DatabaseHandler(const std::string& dbPath) {
        if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
            std::cerr << "Failed to open database: " << sqlite3_errmsg(db) << std::endl;
        } else {
            createTable();
        }
    }

    inline ~DatabaseHandler() {
        sqlite3_close(db);
    }

    inline void logTemperature(double temperature) {
        std::time_t now = std::time(nullptr);
        std::string timestamp = std::ctime(&now);
        timestamp.pop_back();

        std::string query = "INSERT INTO temperatures (timestamp, temperature) VALUES ('" + 
                          timestamp + "', " + std::to_string(temperature) + ");";
        executeQuery(query);
    }

    inline double getCurrentTemperature() {
        sqlite3_stmt* stmt;
        const char* query = "SELECT temperature FROM temperatures ORDER BY timestamp DESC LIMIT 1;";
        
        if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
            return 0.0;
        }

        double result = 0.0;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result = sqlite3_column_double(stmt, 0);
        }

        sqlite3_finalize(stmt);
        return result;
    }

    inline json getTemperatureStats(const std::string& start, const std::string& end) {
        sqlite3_stmt* stmt;
        const char* query = "SELECT AVG(temperature), MIN(temperature), MAX(temperature) "
                           "FROM temperatures WHERE timestamp BETWEEN ? AND ?;";
        
        if (sqlite3_prepare_v2(db, query, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
            return json::object();
        }

        sqlite3_bind_text(stmt, 1, start.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, end.c_str(), -1, SQLITE_STATIC);

        json stats;
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            stats["average"] = sqlite3_column_double(stmt, 0);
            stats["min"] = sqlite3_column_double(stmt, 1);
            stats["max"] = sqlite3_column_double(stmt, 2);
        }

        sqlite3_finalize(stmt);
        return stats;
    }

private:
    sqlite3* db;

    inline void createTable() {
        std::string query = "CREATE TABLE IF NOT EXISTS temperatures (id INTEGER PRIMARY KEY, timestamp TEXT, temperature REAL);";
        executeQuery(query);
    }

    inline void executeQuery(const std::string& query) {
        char* errMsg;
        if (sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
            std::cerr << "SQL error: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
    }
};

// ==================== HttpServer ====================
class HttpServer {
public:
    inline HttpServer(int port, DatabaseHandler& dbHandler) : port(port), dbHandler(dbHandler) {}

    inline void start() {
        httplib::Server server;

        server.Get("/current", [&](const httplib::Request& req, httplib::Response& res) {
            double temperature = dbHandler.getCurrentTemperature();
            json response = {{"temperature", temperature}};
            res.set_content(response.dump(), "application/json");
        });

        server.Get("/stats", [&](const httplib::Request& req, httplib::Response& res) {
            std::string start = req.get_param_value("start");
            std::string end = req.get_param_value("end");
            auto stats = dbHandler.getTemperatureStats(start, end);
            json response = stats;
            res.set_content(response.dump(), "application/json");
        });

        server.listen("0.0.0.0", port);
    }

private:
    int port;
    DatabaseHandler& dbHandler;
};

// ==================== Main ====================
int main() {
    SerialReader serialReader("/dev/pts/5");
    DatabaseHandler dbHandler("temperature.db");
    HttpServer httpServer(8080, dbHandler);

    std::thread httpThread([&httpServer]() {
        httpServer.start();
    });

    while (true) {
        std::string data = serialReader.read();
        if (!data.empty()) {
            double temperature = std::stod(data);
            dbHandler.logTemperature(temperature);
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    httpThread.join();
    return 0;
}