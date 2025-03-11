#include "WorkerThread.h"

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
    //printf("[WorkerThread %d] process tasks",ID_);
    while (!shutdown_) {
        //printf("[WorkerThread %d] looping",ID_);
        auto tq = jobQueue_.lock();
        if (tq && !tq->isShutdown()) {
            //printf("[WorkerThread %d] get task start",ID_);
            auto task = tq->popTask();
            //printf("[WorkerThread %d] get task done",ID_);
            if (task.valid()) {
                //printf("[WorkerThread %d] processing task",ID_);
                task();
            } else {
                //printf(" unable to process task");
            }
        } else {
            // jobQueue_ is not available or destroyed
            // killing the thread
            shutdown_ = true;
        }
    }
}
