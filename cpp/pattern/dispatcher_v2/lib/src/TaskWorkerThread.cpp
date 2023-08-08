
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
   
    std::thread::id id_ = std::this_thread::get_id();
    cout<<"start task thread: "<<id_<<"\n";
    while (!shutdown_) {
  
        //auto tq = jobQueue_.lock();
        //if (tq && !tq->isShutdown()) {
        if (!jobQueue_->isShutdown()) {
      		//cout<<"[task thread] try pop task\n";
            auto task = jobQueue_->popTask();
    		//cout<<"[task thread] get a task\n";
            if (task.valid()) {
                //cout<<"invoke task\n";
                task();
            } else {
         		cout<<"[task thread] task invalid\n";
            }
        } else {
            // jobQueue_ is not available or destroyed
            // killing the thread
            //shutdown_ = true;
        }
    }
    cout<<"exit task worker thread, shutdown="<<shutdown_<<"\n";
}
