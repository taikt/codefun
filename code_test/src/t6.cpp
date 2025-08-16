// File: shared_data.cpp
#include "shared_data.h"
#include <thread>
#include <iostream>

int shared_value = 0;
std::mutex shared_mutex;

void unsafe_increment() {
    // No lock: race condition
    for (int i = 0; i < 10000; ++i) {
        shared_value++;
    }
}

void safe_increment() {
    std::lock_guard<std::mutex> lock(shared_mutex);
    for (int i = 0; i < 10000; ++i) {
        shared_value++;
    }
}

void cross_file_modify() {
    // Simulate cross-file modification
    for (int i = 0; i < 5000; ++i) {
        if (i % 2 == 0) {
            unsafe_increment();
        } else {
            safe_increment();
        }
    }
}
