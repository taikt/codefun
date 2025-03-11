#pragma once
#include <thread>
#include "JobQueue.h"

class WorkerThread {
 public:
    WorkerThread(std::shared_ptr<JobQueue> taskQueue,int ID);
    ~WorkerThread();
    void start();
    bool isShutdown();

 private:
    // process tasks
    void processTasks();

    // weak_ptr for the JobQueue, if task queue is not accessible there is no need to execute
    // tasks
    std::weak_ptr<JobQueue> jobQueue_;

    // flag to indicate stop execution
    std::atomic_bool shutdown_;

    // Thread to run the tasks on
    std::thread thread_;

    int ID_;
};
