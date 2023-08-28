/*
* tai2.tran
*/

#ifndef TASKDISPATCHER_HPP
#define TASKDISPATCHER_HPP

#include <memory>
#include <vector>
#include <map>

#include "JobQueue.h"
#include "TaskWorkerThread.h"
#include "MsgWorkerThread.h"
#include "Handler.h"
#include "Message.h"



namespace kt {

class MsgWorkerThread;
class TaskWorkerThread;

class Dispatcher {
 public:
    Dispatcher(std::shared_ptr<Handler>& handler);
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
    std::map<std::thread::id, std::shared_ptr<std::thread>> dispatchers_;
    // Thread count;
    int threadCount_;
    std::shared_ptr<Handler> handler_;

};

template <typename F, typename... Args>
auto Dispatcher::deliverTask(F task, Args &&... args) -> std::future<decltype(task(args...))> {
    return jobQueue_->pushTask(task, std::forward<Args>(args)...);
}

}

#endif  // THREADPOOL_HPP