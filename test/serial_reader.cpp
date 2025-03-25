#include <fstream>
#include <string>
#include <iostream>

class SerialReader {
public:
    SerialReader(const std::string& port) : port(port) {}

    std::string read() {
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