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
    Dispatcher();
    ~Dispatcher();
    template <typename F, typename... Args>
    auto deliverTask(F task, Args &&... args) -> std::future<decltype(task(args...))>;
    bool deliverMessage(const std::shared_ptr<Message>& message);

    void shutdown();
    bool isShutdown() const;
    void setMessageHandler(std::shared_ptr<Handler>& handler);
    void enableTaskConcurrency(bool m_concurrent);
    void setExpiredTaskTime(uint m_expiredtime); //ms
    void setMaxTaskPoolSize(uint m_maxPoolSize);
    void startTaskThreadPool();
    void startMessageThreadPool();


 private:
    // Queue of tasks
    std::shared_ptr<JobQueue> jobQueue_ = nullptr;
    std::unique_ptr<MsgWorkerThread> msgExecutor_;
    std::unique_ptr<TaskWorkerThread> taskExecutor_;
    
   
    std::shared_ptr<Handler> handler_;

};

template <typename F, typename... Args>
auto Dispatcher::deliverTask(F task, Args &&... args) -> std::future<decltype(task(args...))> {
    return jobQueue_->pushTask(task, std::forward<Args>(args)...);
}

}

#endif  // THREADPOOL_HPP