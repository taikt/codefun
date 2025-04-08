#ifndef TASKDISPATCHER_HPP
#define TASKDISPATCHER_HPP

#include <memory>
#include <vector>

#include "JobQueue.hpp"
#include "WorkerThread.hpp"

class Dispatcher {
 public:
    Dispatcher();
    Dispatcher(int threadCount);
    ~Dispatcher();
    template <typename F, typename... Args>
    auto deliverTask(F task, Args &&... args) -> std::future<decltype(task(args...))>;

    void shutdown();
    bool isShutdown() const;

 private:
    // Queue of tasks
    std::shared_ptr<JobQueue> jobQueue_ = nullptr;

    // Thread count;
    int threadCount_;

    std::vector<std::unique_ptr<WorkerThread>> workerThreads_;
};

template <typename F, typename... Args>
auto Dispatcher::deliverTask(F task, Args &&... args) -> std::future<decltype(task(args...))> {
    return jobQueue_->pushTask(task, std::forward<Args>(args)...);
}

#endif  // THREADPOOL_HPP