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

#include "Log.h"

#include <iostream>
#include <string>
using namespace std;

namespace kt {

class TaskWorkerThread;

class Dispatcher {
 public:
    Dispatcher();
    ~Dispatcher();
    template <typename F, typename... Args>
    auto deliverTask(F task, Args &&... args) -> std::future<decltype(task(args...))>;

    void shutdown();
    bool isShutdown() const;

    void enableTaskConcurrency(bool m_concurrent);
    void setExpiredTaskTime(uint m_expiredtime); //ms
    void setMaxTaskPoolSize(uint m_maxPoolSize);
    void startTaskThreadPool();
    void startMessageThreadPool();


 private:
    // Queue of tasks
    std::shared_ptr<JobQueue> jobQueue_ = nullptr;
    //std::unique_ptr<TaskWorkerThread> taskExecutor_;
    std::shared_ptr<TaskWorkerThread> taskExecutor_;

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