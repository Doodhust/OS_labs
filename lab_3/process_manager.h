#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include "logger.h"
#include "counter.h"
#include <thread>
#include <atomic>

class ProcessManager {
public:
    ProcessManager(Logger& logger, Counter& counter);
    void start();
    void stop();

private:
    Logger& logger;
    Counter& counter;
    std::atomic<bool> running;
    std::thread timerThread;
    std::thread logThread;
    std::thread processThread;

    void timerTask();
    void logTask();
    void processTask();
    void spawnChildProcesses(); // Добавьте это объявление
};

#endif // PROCESS_MANAGER_H