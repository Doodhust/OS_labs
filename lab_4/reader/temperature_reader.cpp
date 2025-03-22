// reader/temperature_reader.cpp
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>
#include <iomanip>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

struct TemperatureData {
    std::string timestamp;
    double temperature;
};

std::string readFromSerialPort(const std::string& port) {
    std::ifstream serial(port);
    std::string data;
    if (serial.is_open()) {
        std::getline(serial, data);
    } else {
        std::cerr << "Failed to open serial port: " << port << std::endl;
    }
    return data;
}

void writeToLog(const std::string& filename, const std::string& data) {
    std::ofstream logfile(filename, std::ios::app);
    if (logfile.is_open()) {
        logfile << data << std::endl;
    } else {
        std::cerr << "Failed to open log file: " << filename << std::endl;
    }
}

double calculateAverage(const std::vector<TemperatureData>& data) {
    double sum = 0.0;
    for (const auto& entry : data) {
        sum += entry.temperature;
    }
    return sum / data.size();
}

int main() {
#ifdef _WIN32
    std::string port = "COM4";  // Для Windows
#else
    std::string port = "/dev/pts/3";  // Для Unix
#endif

    std::vector<TemperatureData> hourlyData;
    std::vector<TemperatureData> dailyData;

    while (true) {
        std::string data = readFromSerialPort(port);
        if (!data.empty()) {
            double temperature = std::stod(data);
            std::time_t now = std::time(nullptr);
            std::string timestamp = std::ctime(&now);
            timestamp.pop_back();

            writeToLog("../logs/all_measurements.log", timestamp + ": " + data);

            TemperatureData entry = {timestamp, temperature};
            hourlyData.push_back(entry);
            dailyData.push_back(entry);

            if (hourlyData.size() >= 60) {
                double hourlyAverage = calculateAverage(hourlyData);
                writeToLog("../logs/hourly_average.log", timestamp + ": " + std::to_string(hourlyAverage));
                hourlyData.clear();
            }

            if (dailyData.size() >= 1440) {
                double dailyAverage = calculateAverage(dailyData);
                writeToLog("../logs/daily_average.log", timestamp + ": " + std::to_string(dailyAverage));
                dailyData.clear();
            }
        }

#ifdef _WIN32
        Sleep(1000);
#else
        sleep(1);
#endif
    }

    return 0;
}