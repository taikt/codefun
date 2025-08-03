/*
* tai2.tran
*/

#include "TaskWorkerThread.h"
#include <iostream>
#include <string>

using namespace std;


namespace kt {

TaskWorkerThread::TaskWorkerThread(std::shared_ptr<JobQueue> taskQueue)
   : jobQueue_(taskQueue)
   , shutdown_(false)
   , work_(std::make_shared<boost::asio::io_service::work>(io_)) {
        //cout<<"taskworkerthread: shutdown="<<shutdown_<<"\n";
   }

TaskWorkerThread::~TaskWorkerThread() {

    shutdown_ = true;
   

    try {
        //std::lock_guard<std::mutex> its_lock(dispatcher_mutex_);
        for (const auto& its_dispatcher : dispatchers_) {
            if (its_dispatcher.second->joinable()) {
                its_dispatcher.second->detach();
            }
        }
        dispatchers_.clear();
    } catch (const std::exception& e) {
        std::cerr << __func__ << " catched exception (dispatchers): " << e.what() << std::endl;
    }
    
    io_threads_.clear(); 
    work_.reset();
    io_.stop();
}

void TaskWorkerThread::start() {
   
    //thread_ = std::thread{[=] { MainProcessTasks(); }};
    
    {
        //std::lock_guard<std::mutex> its_lock(dispatcher_mutex_);
      
        auto its_main_dispatcher = std::make_shared<std::thread>(
                std::bind(&TaskWorkerThread::MainProcessTasks, this));
        dispatchers_[its_main_dispatcher->get_id()] = its_main_dispatcher;
    }

    for (int i = 0; i < 1; i++) {
        std::shared_ptr<std::thread> its_thread = std::make_shared<std::thread>([this, i] {
                                        io_.run();
                                        });
        io_threads_.insert(its_thread);
    }
}

bool TaskWorkerThread::isShutdown() {
    return shutdown_;
}


void TaskWorkerThread::MainProcessTasks() {
   
    std::thread::id id_ = std::this_thread::get_id();
    cout<<"start task main thread: "<<id_<<"\n";
    LOGI("start task main thread:%llu",id_);
    while (!shutdown_) {
        if (!jobQueue_->isShutdown()) {
          
            auto task = jobQueue_->popTask();
           
            if (task.valid()) {
             
                invokeTask(task);
            } else {
                //cout<<"[task thread] task invalid\n";
            }
        } else {
            // jobQueue_ is not available or destroyed
            // killing the thread
            //shutdown_ = true;
        }
    }
    
    cout<<"exit task main thread, shutdown="<<shutdown_<<"\n";
}


void TaskWorkerThread::invokeTask(std::packaged_task<void()>& task) {
    task();
}

}