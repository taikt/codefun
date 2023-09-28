/*
* tai2.tran
*/

#ifndef TASKTHREAD_HPP
#define TASKTHREAD_HPP

#include <thread>
#include "JobQueue.h"

#define DEFAULT_EXPIRED_TIME 1000 //ms
#define DEFAULT_MAX_POOLSIZE 4

namespace kt {

class JobQueue;


class TaskWorkerThread {
 public:
   TaskWorkerThread(std::shared_ptr<JobQueue> taskQueue);
   ~TaskWorkerThread();
   void start();
   bool isShutdown();

   void enableConcurrency(bool m_concurrent); 
   void setExpiredTime(uint m_expiredtime); 
   void setMaxPoolSize(uint m_maxPoolSize); 

 private:
    // process tasks
    void MainProcessTasks();
    void WorkerProcessTasks();
    void invokeTask(std::packaged_task<void()>& task);
    
   

    // weak_ptr for the JobQueue, if task queue is not accessible there is no need to execute
    // tasks
    // std::weak_ptr<JobQueue> jobQueue_;
    std::shared_ptr<JobQueue> jobQueue_;

    // flag to indicate stop execution
    std::atomic_bool shutdown_;
    //int shutdown_;

    // Thread to run the tasks on
    std::thread thread_;


    boost::asio::io_service io_;
    std::shared_ptr<boost::asio::io_service::work> work_;
    std::set<std::shared_ptr<std::thread> > io_threads_;
    std::map<std::thread::id, std::shared_ptr<std::thread>> dispatchers_;
    bool concurrent_ = true;
    uint expiredtime_ = DEFAULT_EXPIRED_TIME;
    uint maxPoolSize_ = DEFAULT_MAX_POOLSIZE;

    int ID_;
};

}

#endif