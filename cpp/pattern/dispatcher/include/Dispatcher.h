#ifndef TASKDISPATCHER_HPP
#define TASKDISPATCHER_HPP

#include <memory>
#include <vector>

#include "JobQueue.h"
#include "TaskWorkerThread.h"
#include "MsgWorkerThread.h"
#include "Message.h"

class Dispatcher {
 public:
    Dispatcher();
    Dispatcher(int threadCount);
    ~Dispatcher();
    template <typename F, typename... Args>
    auto deliverTask(F task, Args &&... args) -> std::future<decltype(task(args...))>;
    bool deliverMessage(const std::shared_ptr<Message>& message);

    void shutdown();
    bool isShutdown() const;

 private:
    // Queue of tasks
    std::shared_ptr<JobQueue> jobQueue_ = nullptr;
    std::vector<std::unique_ptr<MsgWorkerThread>> MsgworkerThreads_;
    std::vector<std::unique_ptr<TaskWorkerThread>> TaskworkerThreads_;

    // Thread count;
    int threadCount_;

};

template <typename F, typename... Args>
auto Dispatcher::deliverTask(F task, Args &&... args) -> std::future<decltype(task(args...))> {
    return jobQueue_->pushTask(task, std::forward<Args>(args)...);
}

#endif  // THREADPOOL_HPP