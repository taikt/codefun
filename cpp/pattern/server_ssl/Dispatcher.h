#pragma once

#include <memory>
#include <vector>
#include "JobQueue.h"
#include "WorkerThread.h"

class Dispatcher {
 public:
    Dispatcher();
    Dispatcher(int inThreadCount, int outThreadCount);
    ~Dispatcher();
    template <typename F, typename... Args>
    auto deliverTaskIn(F task, Args &&... args) -> std::future<decltype(task(args...))>;

    template <typename F, typename... Args>
    auto deliverTaskOut(F task, Args &&... args)->std::future<decltype(task(args...))>;

    void shutdown();
    bool isShutdown() const;

 private:
    // Queue of tasks
    std::shared_ptr<JobQueue> inQueue_ = nullptr;
    std::shared_ptr<JobQueue> outQueue_ = nullptr;

    // Thread count;
    int inThreadCount_;
    int outThreadCount_;

    std::vector<std::unique_ptr<WorkerThread>> inWorkerThreads_;
    std::vector<std::unique_ptr<WorkerThread>> outWorkerThreads_;
};

template <typename F, typename... Args>
auto Dispatcher::deliverTaskIn(F task, Args &&... args) -> std::future<decltype(task(args...))> {
    return inQueue_->pushTask(task, std::forward<Args>(args)...);
}

template <typename F, typename... Args>
auto Dispatcher::deliverTaskOut(F task, Args &&... args) -> std::future<decltype(task(args...))> {
    return outQueue_->pushTask(task, std::forward<Args>(args)...);
}
