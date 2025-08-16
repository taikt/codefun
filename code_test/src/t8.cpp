#include <thread>
#include <mutex>
#include <iostream>

// Global shared resources
int global_counter = 0;
std::mutex global_mutex;

// Unprotected global variable - potential race condition!
int unprotected_data = 42;

class DataManager {
public:
    static DataManager& getInstance() {
        static DataManager instance;  // Thread-unsafe singleton!
        return instance;
    }
    
    void processData() {
        // Race condition: no protection
        unprotected_data++;
        global_counter--;
    }
    
private:
    int internal_data = 0;
};

void thread_worker_safe() {
    std::lock_guard<std::mutex> lock(global_mutex);
    global_counter++;
    std::cout << "Safe access: " << global_counter << std::endl;
}

void thread_worker_unsafe() {
    // Race condition here!
    unprotected_data = unprotected_data * 2;
    global_counter += unprotected_data;
}

int main() {
    std::thread t1(thread_worker_safe);
    std::thread t2(thread_worker_unsafe);
    std::thread t3([]() {
        DataManager::getInstance().processData();
    });
    
    t1.join();
    t2.join(); 
    t3.join();
    
    return 0;
}
