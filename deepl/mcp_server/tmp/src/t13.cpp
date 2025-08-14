#include <thread>
#include <atomic>
#include <vector>

extern int global_counter;  // Shared with other file
std::atomic<int> atomic_counter{0};

class ThreadPool {
private:
    static std::vector<std::thread> workers;
    
public:
    static void addWorker() {
        workers.emplace_back([]() {
            // Access shared resource without protection
            global_counter += 5;
        });
    }
    
    static void processAtomic() {
        atomic_counter.fetch_add(1);
    }
};

std::vector<std::thread> ThreadPool::workers;
