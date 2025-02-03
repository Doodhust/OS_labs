#include "launch_lib.h"
#include <iostream>

int main() {
    const char* command = "ls -l";  // Пример команды для Unix
    bool wait_for_exit = true;

    int result = launch_program(command, wait_for_exit);

    if (result != -1) {
        std::cout << "Command executed with exit code: " << result << std::endl;
    } else {
        std::cout << "Command execution failed!" << std::endl;
    }

    return 0;
}