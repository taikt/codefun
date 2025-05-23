#pragma once
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

class TaskWorkerThread {
public:
    TaskWorkerThread() : running(false) {}
    ~TaskWorkerThread() { stop(); }

    void start() {
        running = true;
        worker = std::thread([this]() { this->run(); });
    }

    void stop() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            running = false;
        }
        cv.notify_one();
        if (worker.joinable())
            worker.join();
    }

    void postTask(const std::function<void()>& task) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            tasks.push(task);
        }
        cv.notify_one();
    }

private:
    void run() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [this] { return !tasks.empty() || !running; });
                if (!running && tasks.empty())
                    break;
                if (!tasks.empty()) {
                    task = std::move(tasks.front());
                    tasks.pop();
                }
            }
            if (task) task();
        }
    }

    std::thread worker;
    std::queue<std::function<void()>> tasks;
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<bool> running;
};