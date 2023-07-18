//#include "Log.h"
#include "TaskWorkerThread.h"
#include <iostream>
#include <string>

using namespace std;

TaskWorkerThread::TaskWorkerThread(std::shared_ptr<JobQueue> taskQueue)
   : jobQueue_(taskQueue)
   , shutdown_(false){
    cout<<"taskworkerthread: shutdown="<<shutdown_<<"\n";
   }

TaskWorkerThread::~TaskWorkerThread() {

    shutdown_ = true;

    if (thread_.joinable()) {
        thread_.join();
    }
}

void TaskWorkerThread::start() {
    thread_ = std::thread{[=] { processTasks(); }};
}

bool TaskWorkerThread::isShutdown() {
    return shutdown_;
}

void TaskWorkerThread::processTasks() {
    cout<<"start task worker thread \n";;
    while (!shutdown_) {
        ////LOGI("[TaskWorkerThread %d] looping",ID_);
        //auto tq = jobQueue_.lock();
        //if (tq && !tq->isShutdown()) {
        if (!jobQueue_->isShutdown()) {
            ////LOGI("[TaskWorkerThread %d] get task start",ID_);
            auto task = jobQueue_->popTask();
            ////LOGI("[TaskWorkerThread %d] get task done",ID_);
            if (task.valid()) {
                //LOGI("[TaskWorkerThread %d] processing task",ID_);
                task();
            } else {
                //LOGI(" unable to process task");
            }
        } else {
            // jobQueue_ is not available or destroyed
            // killing the thread
            //shutdown_ = true;
        }
    }
    cout<<"exit task worker thread, shutdown="<<shutdown_<<"\n";
}
