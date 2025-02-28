#ifndef COUNTER_H
#define COUNTER_H

#include <atomic>
#include <mutex>

class Counter {
public:
    Counter();
    void increment(int value = 1);
    void multiply(int factor);
    void divide(int divisor);
    void set(int value);
    int get();

private:
    std::atomic<int> count; // Атомарный счетчик
    std::mutex mtx; // Мьютекс для синхронизации
};

#endif // COUNTER_H