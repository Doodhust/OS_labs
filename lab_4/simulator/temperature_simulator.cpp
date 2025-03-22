// simulator/temperature_simulator.cpp
#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

void sendToSerialPort(const std::string& port, const std::string& data) {
    std::ofstream serial(port);
    if (serial.is_open()) {
        serial << data << std::endl;
        std::cout << "Sent: " << data << std::endl;
    } else {
        std::cerr << "Failed to open serial port: " << port << std::endl;
    }
}

int main() {
    srand(time(0));

#ifdef _WIN32
    std::string port = "COM3";  // Для Windows
#else
    std::string port = "/dev/pts/2";  // Для Unix
#endif

    while (true) {
        double temperature = 20.0 + (rand() % 100) / 10.0;
        sendToSerialPort(port, std::to_string(temperature));

#ifdef _WIN32
        Sleep(1000);
#else
        sleep(1);
#endif
    }

    return 0;
}