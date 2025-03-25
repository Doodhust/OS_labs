#include <httplib.h>
#include <json.hpp>
#include "database_handler.h"

using json = nlohmann::json;

class HttpServer {
public:
    HttpServer(int port, DatabaseHandler& dbHandler) : port(port), dbHandler(dbHandler) {}

    void start() {
        httplib::Server server;

        server.Get("/current", [&](const httplib::Request& req, httplib::Response& res) {
            // Возвращает текущую температуру
            double temperature = dbHandler.getCurrentTemperature();
            json response = {{"temperature", temperature}};
            res.set_content(response.dump(), "application/json");
        });

        server.Get("/stats", [&](const httplib::Request& req, httplib::Response& res) {
            // Возвращает статистику за период
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