#include "process_manager.h"
#include <unistd.h>
#include <sys/wait.h>
#include <chrono>
#include <thread>
#include <iostream>
#include <atomic>

ProcessManager::ProcessManager(Logger& logger, Counter& counter)
    : logger(logger), counter(counter), running(false) {}

void ProcessManager::start() {
    running = true;
    timerThread = std::thread(&ProcessManager::timerTask, this);
    logThread = std::thread(&ProcessManager::logTask, this);
    processThread = std::thread(&ProcessManager::processTask, this);
}

void ProcessManager::stop() {
    running = false;
    if (timerThread.joinable()) timerThread.join();
    if (logThread.joinable()) logThread.join();
    if (processThread.joinable()) processThread.join();
}

void ProcessManager::timerTask() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        counter.increment();
    }
}

void ProcessManager::logTask() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        int count = counter.get();
        logger.log("PID: " + std::to_string(getpid()) + " Counter: " + std::to_string(count));
    }
}

void ProcessManager::processTask() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        spawnChildProcesses();
    }
}

void ProcessManager::spawnChildProcesses() {
    pid_t pid1 = fork();
    if (pid1 == 0) {
        // Копия 1
        logger.log("Copy 1 started. PID: " + std::to_string(getpid()));
        counter.increment(10); // Увеличиваем счетчик на 10
        logger.log("Copy 1 finished. PID: " + std::to_string(getpid()));
        exit(0); // Завершаем процесс
    } else if (pid1 > 0) {
        pid_t pid2 = fork();
        if (pid2 == 0) {
            // Копия 2
            logger.log("Copy 2 started. PID: " + std::to_string(getpid()));
            counter.multiply(2); // Умножаем счетчик на 2
            logger.log("Counter multiplied by 2. New value: " + std::to_string(counter.get()));
            std::this_thread::sleep_for(std::chrono::seconds(2)); // Ждем 2 секунды
            counter.divide(2); // Делим счетчик на 2
            logger.log("Counter divided by 2. New value: " + std::to_string(counter.get()));
            logger.log("Copy 2 finished. PID: " + std::to_string(getpid()));
            exit(0); // Завершаем процесс
        } else if (pid2 > 0) {
            // Родительский процесс
            int status;
            waitpid(pid1, &status, 0); // Ждем завершения копии 1
            waitpid(pid2, &status, 0); // Ждем завершения копии 2
        } else {
            logger.log("Failed to create Copy 2");
        }
    } else {
        logger.log("Failed to create Copy 1");
    }
}