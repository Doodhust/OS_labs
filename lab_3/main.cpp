#include "logger.h"
#include "counter.h"
#include "process_manager.h"
#include <iostream>
#include <thread>
#include <atomic>

Logger logger("log.txt"); // Логгер
Counter counter; // Счетчик

int main() {
    ProcessManager manager(logger, counter);
    manager.start();

    std::string input;
    while (true) {
        std::cin >> input;
        if (input == "set") {
            int value;
            std::cin >> value;
            counter.set(value); // Используйте метод set из класса Counter
        } else if (input == "exit") {
            break;
        }
    }

    manager.stop();
    return 0;
}