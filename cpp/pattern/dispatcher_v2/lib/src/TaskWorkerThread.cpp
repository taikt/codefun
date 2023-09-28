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

    if (thread_.joinable()) {
        thread_.join();
    }

    io_threads_.clear(); 
    dispatchers_.clear();
    
    work_.reset();
    io_.stop();
}

void TaskWorkerThread::start() {
    thread_ = std::thread{[=] { MainProcessTasks(); }};

    for (int i = 0; i < 3; i++) {
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
    while (!shutdown_) {
  
        //auto tq = jobQueue_.lock();
        //if (tq && !tq->isShutdown()) {
        if (!jobQueue_->isShutdown()) {
      		//cout<<"[task thread] try pop task\n";
            auto task = jobQueue_->popTask();
    		//cout<<"[task thread] get a task\n";
            if (task.valid()) {
                //cout<<"invoke task\n";
                invokeTask(task);
            } else {
         		cout<<"[task thread] task invalid\n";
            }
        } else {
            // jobQueue_ is not available or destroyed
            // killing the thread
            //shutdown_ = true;
        }
    }
    cout<<"exit task main thread, shutdown="<<shutdown_<<"\n";
}

void TaskWorkerThread::WorkerProcessTasks() {
   
    std::thread::id id_ = std::this_thread::get_id();
    cout<<"start task worker thread: "<<id_<<"\n";
    while (jobQueue_->getTaskQueueSize() > 0) {
  
        //auto tq = jobQueue_.lock();
        //if (tq && !tq->isShutdown()) {
        if (!jobQueue_->isShutdown()) {
      		//cout<<"[task thread] try pop task\n";
            auto task = jobQueue_->popTask();
    		//cout<<"[task thread] get a task\n";
            if (task.valid()) {
                //cout<<"invoke task\n";
                invokeTask(task);
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


void TaskWorkerThread::invokeTask(std::packaged_task<void()>& task) {
    boost::asio::steady_timer its_dispatcher_timer(io_);
    if (concurrent_) {
        its_dispatcher_timer.expires_from_now(std::chrono::milliseconds(expiredtime_)); // default 1sec
        its_dispatcher_timer.async_wait([this](const boost::system::error_code &_error) {
            if (!_error) {
                if (dispatchers_.size() < maxPoolSize_) {
                        cout<<"start new task dispatcher\n";
                        auto its_dispatcher = std::make_shared<std::thread>(
                            std::bind(&TaskWorkerThread::WorkerProcessTasks, this));
                        dispatchers_[its_dispatcher->get_id()] = its_dispatcher;
                
                } else {
                    cout << "Maximum number of dispatchers exceeded.\n";
                }          
            }
        });
    }

    task();
    
    if (concurrent_) {
        boost::system::error_code ec;
        its_dispatcher_timer.cancel(ec);
    }
}

void TaskWorkerThread::enableConcurrency(bool m_concurrent) {
    concurrent_ = m_concurrent;
}

void TaskWorkerThread::setExpiredTime(uint m_expiredtime) { //ms
    expiredtime_ = m_expiredtime;
}

void TaskWorkerThread::setMaxPoolSize(uint m_maxPoolSize) { //ms
    maxPoolSize_ = m_maxPoolSize;
}

}