// File: thread_worker.cpp
#include "shared_data.h"
#include <thread>
#include <iostream>

void thread_func1() {
    for (int i = 0; i < 10; ++i) {
        unsafe_increment();
    }
}

void thread_func2() {
    for (int i = 0; i < 10; ++i) {
        safe_increment();
    }
}

void thread_func3() {
    cross_file_modify();
}

void run_threads() {
    std::thread t1(thread_func1);
    std::thread t2(thread_func2);
    std::thread t3(thread_func3);
    t1.join();
    t2.join();
    t3.join();
    std::cout << "Final shared_value: " << shared_value << std::endl;
}
