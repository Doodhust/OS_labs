// server/temperature_server.cpp
#include <iostream>
#include <thread>
#include "serial_reader.h"
#include "database_handler.h"
#include "http_server.h"

int main() {
    // Инициализация компонентов
    SerialReader serialReader("/dev/pts/3");  // Для Linux
    DatabaseHandler dbHandler("temperature.db");
    HttpServer httpServer(8080, dbHandler);

    // Запуск HTTP-сервера в отдельном потоке
    std::thread httpThread([&httpServer]() {
        httpServer.start();
    });

    // Основной цикл чтения данных с порта
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