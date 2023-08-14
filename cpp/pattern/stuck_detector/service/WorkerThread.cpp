#include "Log.h"
#include "WorkerThread.hpp"
#define LOG_TAG "WorkerThread"



WorkerThread::WorkerThread(std::shared_ptr<JobQueue> taskQueue, int ID)
   : jobQueue_(taskQueue)
   , shutdown_(false)
   , ID_(ID) {

}

WorkerThread::~WorkerThread() {

    shutdown_ = true;

    if (thread_.joinable()) {
        thread_.join();
    }
}

void WorkerThread::start() {
    thread_ = std::thread{[=] { processTasks(); }};
}

bool WorkerThread::isShutdown() {
    return shutdown_;
}

void WorkerThread::processTasks() {
    //LOGI("[WorkerThread %d] process tasks",ID_);
    while (!shutdown_) {
        //LOGI("[WorkerThread %d] looping",ID_);
        auto tq = jobQueue_.lock();
        if (tq && !tq->isShutdown()) {
            //LOGI("[WorkerThread %d] get task start",ID_);
            auto task = tq->popTask();
            //LOGI("[WorkerThread %d] get task done",ID_);
            if (task.valid()) {
                LOGI("[WorkerThread %d] processing task",ID_);
                task();
            } else {
                LOGI(" unable to process task");
            }
        } else {
            // jobQueue_ is not available or destroyed
            // killing the thread
            shutdown_ = true;
        }
    }
}
