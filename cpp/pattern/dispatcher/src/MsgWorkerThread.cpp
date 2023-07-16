//#include "Log.h"
#include "MsgWorkerThread.h"
#include <iostream>
#include <string>

using namespace std;



MsgWorkerThread::MsgWorkerThread(std::shared_ptr<JobQueue> taskQueue)
   : jobQueue_(taskQueue),
   shutdown_(false)
   {
    cout<<"msgworkerthread: shutdown="<<shutdown_<<"\n";
   }

MsgWorkerThread::~MsgWorkerThread() {

    shutdown_ = true;

    if (thread_.joinable()) {
        thread_.join();
    }
}

void MsgWorkerThread::start() {
    thread_ = std::thread(&MsgWorkerThread::processMessage,this);
}

bool MsgWorkerThread::isShutdown() {
    return shutdown_;
}

void MsgWorkerThread::processMessage() {
    cout<<"start msg worker thread: shutdown="<<shutdown_<<"\n";
    while (!shutdown_) {
        ////LOGI("[MsgWorkerThread %d] looping",ID_);
        //auto tq = jobQueue_.lock();
        //if (tq && !tq->isShutdown()) {
        if (!jobQueue_->isShutdown()) {
            ////LOGI("[MsgWorkerThread %d] get task start",ID_);
            auto msg = jobQueue_->popMessage();
            handleMessage(msg);
        } else {
            // jobQueue_ is not available or destroyed
            // killing the thread
            // shutdown_ = true;
        }
    }
    cout<<"exit msg worker thread\n";
}

void MsgWorkerThread::handleMessage(std::shared_ptr<Message>& msg) {
    cout<<"handle a message: id="<<msg->id<<", value:"<<msg->str<<"\n";
}
