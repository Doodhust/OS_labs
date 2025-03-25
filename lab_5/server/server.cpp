#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <ctime>
#include <cstdlib>
#include <random>
#include <sqlite3.h>
#include <httplib.h>
#include <nlohmann/json.hpp>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

using namespace std;
using json = nlohmann::json;

// ==================== Платформозависимые функции ====================
string get_default_serial_port() {
    #ifdef _WIN32
        return "COM3";
    #else
        return "/dev/2";
    #endif
}

void sleep_ms(int milliseconds) {
    #ifdef _WIN32
        Sleep(milliseconds);
    #else
        usleep(milliseconds * 1000);
    #endif
}

// ==================== SerialReader ====================
class SerialReader {
public:
    SerialReader(const string& port) : port(port) {
        cout << "Initializing SerialReader with port: " << port << endl;
    }

    string read() {
        #ifdef _WIN32
        // Реализация для Windows
        HANDLE hSerial = CreateFile(port.c_str(), GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
        if (hSerial == INVALID_HANDLE_VALUE) {
            cerr << "Error: Failed to open serial port: " << port << endl;
            return "";
        }

        DCB dcbSerialParams = {0};
        dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
        if (!GetCommState(hSerial, &dcbSerialParams)) {
            cerr << "Error getting port state" << endl;
            CloseHandle(hSerial);
            return "";
        }

        dcbSerialParams.BaudRate = CBR_9600;
        dcbSerialParams.ByteSize = 8;
        dcbSerialParams.StopBits = ONESTOPBIT;
        dcbSerialParams.Parity = NOPARITY;
        if(!SetCommState(hSerial, &dcbSerialParams)) {
            cerr << "Error setting port state" << endl;
            CloseHandle(hSerial);
            return "";
        }

        char buffer[256];
        DWORD bytesRead;
        if (!ReadFile(hSerial, buffer, sizeof(buffer), &bytesRead, NULL)) {
            cerr << "Error reading from port" << endl;
            CloseHandle(hSerial);
            return "";
        }

        CloseHandle(hSerial);
        return string(buffer, bytesRead);

        #else
        // Реализация для Linux
        ifstream serial(port);
        if (!serial.is_open()) {
            cerr << "Error: Failed to open serial port: " << port << endl;
            return "";
        }

        string data;
        if (!getline(serial, data)) {
            cerr << "Warning: No data received from port: " << port << endl;
            return "";
        }

        cout << "Received raw data: '" << data << "'" << endl;
        return data;
        #endif
    }

private:
    string port;
};

// ==================== DatabaseHandler ====================
class DatabaseHandler {
public:
    DatabaseHandler(const string& dbPath) : dbPath(dbPath) {
        if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
            cerr << "Error: Can't open database: " << sqlite3_errmsg(db) << endl;
            exit(1);
        }
        createTable();
        cout << "Database initialized: " << dbPath << endl;
    }

    ~DatabaseHandler() {
        sqlite3_close(db);
    }

    void logTemperature(double temperature) {
        char timestamp[20];
        time_t now = time(nullptr);
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

        string query = "INSERT INTO temperatures (timestamp, temperature) VALUES (?, ?);";
        sqlite3_stmt* stmt;
        
        if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            cerr << "Error preparing statement: " << sqlite3_errmsg(db) << endl;
            return;
        }

        sqlite3_bind_text(stmt, 1, timestamp, -1, SQLITE_STATIC);
        sqlite3_bind_double(stmt, 2, temperature);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            cerr << "Error inserting data: " << sqlite3_errmsg(db) << endl;
        }

        sqlite3_finalize(stmt);
        cout << "Logged temperature: " << temperature << "°C at " << timestamp << endl;
    }

    double getCurrentTemperature() {
        string query = "SELECT temperature FROM temperatures ORDER BY timestamp DESC LIMIT 1;";
        sqlite3_stmt* stmt;
        double result = 0.0;

        if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            cerr << "Error preparing statement: " << sqlite3_errmsg(db) << endl;
            return result;
        }

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            result = sqlite3_column_double(stmt, 0);
        } else {
            cout << "No temperature data found in database" << endl;
        }

        sqlite3_finalize(stmt);
        return result;
    }

    json getTemperatureStats(const string& start, const string& end) {
        json stats = {{"average", 0.0}, {"min", 0.0}, {"max", 0.0}, {"count", 0}};
        string query = R"(
            SELECT 
                AVG(temperature), 
                MIN(temperature), 
                MAX(temperature),
                COUNT(*) 
            FROM temperatures 
            WHERE timestamp BETWEEN ? AND ?
        )";

        sqlite3_stmt* stmt;
        if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            cerr << "Error preparing statement: " << sqlite3_errmsg(db) << endl;
            return stats;
        }

        sqlite3_bind_text(stmt, 1, start.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, end.c_str(), -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            stats["average"] = sqlite3_column_double(stmt, 0);
            stats["min"] = sqlite3_column_double(stmt, 1);
            stats["max"] = sqlite3_column_double(stmt, 2);
            stats["count"] = sqlite3_column_int(stmt, 3);
        }

        sqlite3_finalize(stmt);
        return stats;
    }

private:
    sqlite3* db;
    string dbPath;

    void createTable() {
        const char* query = R"(
            CREATE TABLE IF NOT EXISTS temperatures (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                timestamp TEXT NOT NULL,
                temperature REAL NOT NULL
            );
        )";

        char* errMsg = nullptr;
        if (sqlite3_exec(db, query, nullptr, nullptr, &errMsg) != SQLITE_OK) {
            cerr << "Error creating table: " << errMsg << endl;
            sqlite3_free(errMsg);
            exit(1);
        }
    }
};

class HttpServer {
public:
    HttpServer(int port, DatabaseHandler& db) : port(port), db(db) {}

    void start() {
        server.Get("/current", [&](const httplib::Request&, httplib::Response& res) {
            double temp = db.getCurrentTemperature();
            json response = {{"temperature", temp}, {"unit", "Celsius"}};
            res.set_content(response.dump(), "application/json");
            cout << "Served current temperature: " << temp << "°C" << endl;
        });

        server.Get("/stats", [&](const httplib::Request& req, httplib::Response& res) {
            string start = req.has_param("start") ? req.get_param_value("start") : "1970-01-01";
            string end = req.has_param("end") ? req.get_param_value("end") : "2100-01-01";
            
            auto stats = db.getTemperatureStats(start, end);
            res.set_content(stats.dump(), "application/json");
            
            cout << "Served stats from " << start << " to " << end 
                 << ": avg=" << stats["average"] << ", min=" << stats["min"] 
                 << ", max=" << stats["max"] << endl;
        });

        cout << "Starting HTTP server on port " << port << endl;
        server.listen("0.0.0.0", port);
    }

private:
    int port;
    DatabaseHandler& db;
    httplib::Server server;
};

int main() {
    cout << "Starting temperature monitoring system..." << endl;

    // Кроссплатформенные настройки
    const string serial_port = get_default_serial_port();
    const string db_file = "temperature.db";
    const int http_port = 8080;

    // Инициализация компонентов
    DatabaseHandler db(db_file);
    HttpServer server(http_port, db);

    // Запуск HTTP сервера в отдельном потоке
    thread server_thread([&server]() {
        server.start();
    });

    // Основной цикл сбора данных
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(20.0, 30.0);

    // Инициализируем SerialReader
    try {
        SerialReader reader(serial_port);

        while (true) {
            string data = reader.read();
            double temperature = 0.0;

            if (!data.empty()) {
                try {
                    temperature = stod(data);
                    cout << "Parsed temperature: " << temperature << "°C" << endl;
                } catch (const exception& e) {
                    cerr << "Error parsing data: " << e.what() 
                         << " (received: '" << data << "')" << endl;
                    temperature = dis(gen);
                    cout << "Using generated temperature: " << temperature << "°C" << endl;
                }
            } else {
                temperature = dis(gen);
                cout << "No data received, using generated: " << temperature << "°C" << endl;
            }

            db.logTemperature(temperature);
            sleep_ms(1000);
        }
    } catch (const exception& e) {
        cerr << "Serial port initialization failed: " << e.what() << endl;
        cerr << "Running in simulation mode with random data" << endl;

        while (true) {
            double temperature = dis(gen);
            db.logTemperature(temperature);
            cout << "Generated temperature: " << temperature << "°C" << endl;
            sleep_ms(1000);
        }
    }

    server_thread.join();
    return 0;
}