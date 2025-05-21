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
    is_dispatching_ = false;

    try {
        std::lock_guard<std::mutex> its_lock(dispatcher_mutex_);
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
    is_dispatching_ = true;
    //thread_ = std::thread{[=] { MainProcessTasks(); }};
    
    {
        std::lock_guard<std::mutex> its_lock(dispatcher_mutex_);
        is_dispatching_ = true;
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
    boost::asio::steady_timer its_dispatcher_timer(io_);
    std::thread::id id_ = std::this_thread::get_id();

    if (concurrent_) {
        its_dispatcher_timer.expires_from_now(std::chrono::milliseconds(expiredtime_)); // default 1sec
        its_dispatcher_timer.async_wait([this](const boost::system::error_code &_error) {
            if (!_error) {
                if (has_active_dispatcher()) {
                    //cout<<"find a active dispatcher\n";
                    std::lock_guard<std::mutex> its_lock(handlers_mutex_);
                    dispatcher_condition_.notify_all();
                }else {
                    //cout<<"no active dispatcher, create new one\n";
                    while (!shutdown_) {
                        if (dispatcher_mutex_.try_lock()) {
                            if (dispatchers_.size() < maxPoolSize_) {
                                cout<<"start new task dispatcher\n";
                                LOGI("start new task dispatcher");
                                auto its_dispatcher = std::make_shared<std::thread>(std::bind(&TaskWorkerThread::WorkerProcessTasks, this));
                                dispatchers_[its_dispatcher->get_id()] = its_dispatcher;
                            } else {
                                cout << "Maximum number of dispatchers exceeded.\n";
                                LOGI("Maximum number of dispatchers exceeded");
                            }
                            dispatcher_mutex_.unlock();
                            break;

                        } else {
                            std::this_thread::yield();
                        }
                    }
                }
            }
        });
    }

    while (concurrent_ && !shutdown_ ) {
        if (dispatcher_mutex_.try_lock()) {
            // taikt: add thread of current dispacher to running list before truely doing job
            //cout<<"add me to running list\n";
            running_dispatchers_.insert(id_);
            dispatcher_mutex_.unlock();
            break;
        }
        std::this_thread::yield();
    }

    task();
    
    if (concurrent_) {
        boost::system::error_code ec;
        its_dispatcher_timer.cancel(ec);
    }

    while (concurrent_ && !shutdown_ ) {
        if (dispatcher_mutex_.try_lock()) {
            // taikt: current dispacher finish job on time, remove it from the running list
            //cout<<"free me from running list\n";
            running_dispatchers_.erase(id_);
            dispatcher_mutex_.unlock();
            return;
        }
        std::this_thread::yield();
    }
}

bool TaskWorkerThread::has_active_dispatcher() {
    while (!shutdown_) {
        if (dispatcher_mutex_.try_lock()) {
            for (const auto &d : dispatchers_) { // check every worker dispatcher in pool (list of dispachers_)
                //d.first: thread id of worker dispatcher, d.second: thread running worker dispatcher
                if (running_dispatchers_.find(d.first) == running_dispatchers_.end() &&  // not found thread id of worker dispatcher in running_dispatcher
                    elapsed_dispatchers_.find(d.first) == elapsed_dispatchers_.end()) { // not found thread id of worker dispatcher in elapsed_dispatcher
                    dispatcher_mutex_.unlock();
                    return true;
                }
            }
            dispatcher_mutex_.unlock();
            return false;
        }
        std::this_thread::yield();
    }
    return false;
}

// taikt: return true if cannot find any other free dispatcher, meaning this dispacher need to do job 
// (it found a differrent free dispatcher in pool -> return false)
bool TaskWorkerThread::is_active_dispatcher(const std::thread::id &_id) {
    /*
    LOGI("is_active_dispatcher: dispatchers list");
    // dbuggi
    for (auto el: dispatchers_) {
        cout<<"worker:"<<el.first<<"\n";
        LOGI("worker:%llu",el.first);
    }
    */
    while (!shutdown_) {
        if (dispatcher_mutex_.try_lock()) {
            for (const auto &d : dispatchers_) { 
                if (d.first != _id && // checked worker dispatcher differs current thread
                    running_dispatchers_.find(d.first) == running_dispatchers_.end() && // checked worker dispatcher not in running list
                    elapsed_dispatchers_.find(d.first) == elapsed_dispatchers_.end()) { // checked worker dispatcher not in elapsed list
                    dispatcher_mutex_.unlock();
                    //cout<<"not active dispatcher\n";
                    //LOGI("not active dispatcher, d.first=%llu, _id=%llu",d.first, _id);
                    return false; // if there is a free dispatcher in pool, which differ with current thread => return fail (this thread cannot do job, and will be moved to elapsed list)
                                  // this to prevent case that: current thread has to take a lot of job, meanwhile other free worker do nothing
                                  // to provide balance between workers? 
                }
            }
            dispatcher_mutex_.unlock();
            //cout<<"active dispatcher\n";
            //LOGI("active dispatcher");
            return true; // taikt: if every woker dispatcher in pool is busy, return true (this thread must do job)
        }
        std::this_thread::yield();
    }
    //cout<<"not active dispatcher2\n";
    //LOGI("not active dispatcher2");
    return false;
}

void TaskWorkerThread::remove_elapsed_dispatchers() {
    LOGI("remove_elapsed_dispatchers");
    if (!shutdown_) {
        std::lock_guard<std::mutex> its_lock(dispatcher_mutex_);
        for (auto id : elapsed_dispatchers_) {
            auto its_dispatcher = dispatchers_.find(id);
            if (its_dispatcher->second->joinable())
                its_dispatcher->second->join();
           // cout<<"remove elapsed worker:"<<its_dispatcher->first<<"\n";
            dispatchers_.erase(id);
        }
        elapsed_dispatchers_.clear();
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

void TaskWorkerThread::TimerHandler( const boost::system::error_code & error, boost::shared_ptr< boost::asio::deadline_timer > timer)
{
    if( error )
    {
        //std::thread::id id_ = std::this_thread::get_id();
        cout << "[" << boost::this_thread::get_id()<< "] timer Error: " << error << "\n";  
    }
    else
    {

        cout << "[" << boost::this_thread::get_id()<< "] TimerHandler: running " << "\n";
        cout<<"********************\n";
        cout<<"dispatchers list\n";
        LOGI("dispatchers list");
        for (auto el: dispatchers_) {
            cout<<"worker:"<<el.first<<"\n";
            LOGI("worker:%llu",el.first);
        }

        cout<<"running list\n";
        LOGI("running list");
        for (auto el: running_dispatchers_) {
            cout<<"worker:"<<el<<"\n";
            LOGI("worker:%llu",el);
        }

        cout<<"elapsed list\n";
        LOGI("elapsed list");
        for (auto el: elapsed_dispatchers_) {
            cout<<"worker:"<<el<<"\n";
            LOGI("worker:%llu",el);
        }
        cout<<"********************\n";

        timer->expires_from_now( boost::posix_time::seconds(1));
        timer->async_wait( boost::bind( &TaskWorkerThread::TimerHandler, this, _1, timer ) );
    }
}

}