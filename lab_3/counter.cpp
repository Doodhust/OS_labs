#include "counter.h"

Counter::Counter() : count(0) {}

void Counter::increment(int value) {
    std::lock_guard<std::mutex> lock(mtx);
    count += value;
}

void Counter::multiply(int factor) {
    std::lock_guard<std::mutex> lock(mtx);
    int currentValue = count.load(); // Чтение текущего значения
    count.store(currentValue * factor); // Запись нового значения
}

void Counter::divide(int divisor) {
    std::lock_guard<std::mutex> lock(mtx);
    if (divisor != 0) {
        int currentValue = count.load(); // Чтение текущего значения
        count.store(currentValue / divisor); // Запись нового значения
    }
}

void Counter::set(int value) {
    std::lock_guard<std::mutex> lock(mtx);
    count.store(value); // Запись нового значения
}

int Counter::get() {
    std::lock_guard<std::mutex> lock(mtx);
    return count.load(); // Чтение текущего значения
}