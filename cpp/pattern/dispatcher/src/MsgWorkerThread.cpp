#include "MsgWorkerThread.h"

#include <iostream>
#include <string>
using namespace std;



MsgWorkerThread::MsgWorkerThread(std::shared_ptr<JobQueue> taskQueue, std::shared_ptr<Handler> handler)
   : jobQueue_(taskQueue),
   shutdown_(false),
   handler_(handler),
   work_(std::make_shared<boost::asio::io_service::work>(io_))
   {
        cout<<"msgworkerthread: shutdown="<<shutdown_<<"\n";
   }

MsgWorkerThread::~MsgWorkerThread() {

    shutdown_ = true;
    io_threads_.clear();
    if (thread_.joinable()) {
        thread_.join();
    }

    dispatchers_.clear();
    

    work_.reset();
    io_.stop();
}

void MsgWorkerThread::start() {
    thread_ = std::thread(&MsgWorkerThread::mainMessageDispatcher,this);
    for (int i = 0; i < 2; i++) {
        std::shared_ptr<std::thread> its_thread = std::make_shared<std::thread>([this, i] {
                                        io_.run();
                                        });
        io_threads_.insert(its_thread);
    }
 
}

bool MsgWorkerThread::isShutdown() {
    return shutdown_;
}

void MsgWorkerThread::mainMessageDispatcher() {
    std::thread::id id_ = std::this_thread::get_id();
    cout<<"start main msg worker thread: "<<id_<<"\n";
    while (!shutdown_) {
        //auto tq = jobQueue_.lock();
        //if (tq && !tq->isShutdown()) {
        if (!jobQueue_->isShutdown()) {
           
            auto msg = jobQueue_->popMessage();
            invokeHandler(msg);
            //handler_->handleMessage(msg);
        } else {
            // jobQueue_ is not available or destroyed
            // killing the thread
            // shutdown_ = true;
        }
    }
    cout<<"exit main msg worker thread\n";
}

void MsgWorkerThread::messageDispatcher() {
    std::thread::id id_ = std::this_thread::get_id();
    cout<<"start new msg worker thread: "<<id_<<"\n";
    while (jobQueue_->getMsgQueueSize() > 0) {
        //auto tq = jobQueue_.lock();
        //if (tq && !tq->isShutdown()) {
        if (!jobQueue_->isShutdown()) {
           
            auto msg = jobQueue_->popMessage();
            invokeHandler(msg);
        } else {
            // jobQueue_ is not available or destroyed
            // killing the thread
            // shutdown_ = true;
        }
    }
    cout<<"exit msg worker thread:"<<id_<<"\n";
}

void MsgWorkerThread::invokeHandler(std::shared_ptr<Message>& msg) {
    //cout<<"invoke handler\n";
    boost::asio::steady_timer its_dispatcher_timer(io_);
    its_dispatcher_timer.expires_from_now(std::chrono::milliseconds(1000)); // default 1sec
    its_dispatcher_timer.async_wait([this](const boost::system::error_code &_error) {
        if (!_error) {
            if (dispatchers_.size() < 3) {
                    cout<<"start new dispatcher\n";
                    auto its_dispatcher = std::make_shared<std::thread>(
                        std::bind(&MsgWorkerThread::messageDispatcher, this));
                    dispatchers_[its_dispatcher->get_id()] = its_dispatcher;
               
            } else {
                cout << "Maximum number of dispatchers exceeded.\n";
            }          
        }
    });
         

    handler_->handleMessage(msg);

    boost::system::error_code ec;
    its_dispatcher_timer.cancel(ec);
}

void MsgWorkerThread::handleMessage(std::shared_ptr<Message>& msg) {
    //cout<<"handle a message: id="<<msg->id<<", value:"<<msg->str<<"\n";
}
