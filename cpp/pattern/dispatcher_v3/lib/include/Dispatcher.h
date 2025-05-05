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
#include "Log.h"

#include <iostream>
#include <string>
using namespace std;

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
    //std::unique_ptr<TaskWorkerThread> taskExecutor_;
    std::shared_ptr<TaskWorkerThread> taskExecutor_;
    
   
    std::shared_ptr<Handler> handler_;

};

template <typename F, typename... Args>
auto Dispatcher::deliverTask(F task, Args &&... args) -> std::future<decltype(task(args...))> {
   
    // only unblock a waiting thread, not unblock all waiting thread

    auto future = jobQueue_->pushTask(task, std::forward<Args>(args)...);
    LOGI("[dispatcher] alread submit task, current size=%d",jobQueue_->getTaskQueueSize());
    
    return future;
}

}

#endif  // THREADPOOL_HPP