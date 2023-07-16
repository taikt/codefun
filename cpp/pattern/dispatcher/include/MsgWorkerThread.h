#ifndef MSGTASKTHREAD_HPP
#define MSGTASKTHREAD_HPP

#include <thread>
#include "JobQueue.h"

class MsgWorkerThread {
 public:
    MsgWorkerThread(std::shared_ptr<JobQueue> taskQueue);
    ~MsgWorkerThread();
    void start();
    bool isShutdown();


 private:
    // process tasks
    void processMessage();
    void handleMessage(std::shared_ptr<Message>& msg);
    // weak_ptr for the JobQueue, if task queue is not accessible there is no need to execute
    // tasks
    //std::weak_ptr<JobQueue> jobQueue_;
    std::shared_ptr<JobQueue> jobQueue_;

    // flag to indicate stop execution
    std::atomic_bool shutdown_;
    //int shutdown;

    // Thread to run the tasks on
    std::thread thread_;
};


#endif