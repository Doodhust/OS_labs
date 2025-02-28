#include "logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <ctime>

Logger::Logger(const std::string& filename) {
    logFile.open(filename, std::ios::app);
    if (!logFile.is_open()) {
        std::cerr << "Failed to open log file!" << std::endl;
    }
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

void Logger::log(const std::string& message) {
    if (logFile.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto now_time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        logFile << std::put_time(std::localtime(&now_time), "%Y-%m-%d %H:%M:%S.")
                << std::setfill('0') << std::setw(3) << ms.count() << " " << message << std::endl;
    }
}