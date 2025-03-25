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

namespace std;

struct TemperatureData {
    string timestamp;
    double temperature;
};

string readFromSerialPort(const string& port) {
    ifstream serial(port);
    string data;
    if (serial.is_open()) {
        getline(serial, data);
    } else {
        cerr << "Failed to open serial port: " << port << endl;
    }
    return data;
}

void writeToLog(const string& filename, const string& data) {
    ofstream logfile(filename, ios::app);
    if (logfile.is_open()) {
        logfile << data << endl;
    } else {
        cerr << "Failed to open log file: " << filename << endl;
    }
}

double calculateAverage(const vector<TemperatureData>& data) {
    double sum = 0.0;
    for (const auto& entry : data) {
        sum += entry.temperature;
    }
    return sum / data.size();
}

int main() {
#ifdef _WIN32
    string port = "COM4";  // Для Windows
#else
    string port = "/dev/pts/3";  // Для Unix
#endif

    vector<TemperatureData> hourlyData;
    vector<TemperatureData> dailyData;

    while (true) {
        string data = readFromSerialPort(port);
        if (!data.empty()) {
            double temperature = stod(data);
            time_t now = time(nullptr);
            string timestamp = ctime(&now);
            timestamp.pop_back();

            writeToLog("../logs/all_measurements.log", timestamp + ": " + data);

            TemperatureData entry = {timestamp, temperature};
            hourlyData.push_back(entry);
            dailyData.push_back(entry);

            if (hourlyData.size() >= 60) {
                double hourlyAverage = calculateAverage(hourlyData);
                writeToLog("../logs/hourly_average.log", timestamp + ": " + to_string(hourlyAverage));
                hourlyData.clear();
            }

            if (dailyData.size() >= 1440) {
                double dailyAverage = calculateAverage(dailyData);
                writeToLog("../logs/daily_average.log", timestamp + ": " + to_string(dailyAverage));
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