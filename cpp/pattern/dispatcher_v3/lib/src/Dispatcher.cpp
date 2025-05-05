/*
* tai2.tran
*/

#include "Dispatcher.h"
#include <iostream>
#include <string>

using namespace std;

namespace kt {

Dispatcher::Dispatcher()
   : jobQueue_{std::make_shared<JobQueue>()}
{
    LOGI("start dispatcher");
    taskExecutor_ = std::make_shared<TaskWorkerThread>(jobQueue_);
    jobQueue_->setTaskExecutor(taskExecutor_);
}

Dispatcher::~Dispatcher() {
    shutdown();
}

void Dispatcher::shutdown() {
    cout<<"dispatcher shutdown..\n";
    jobQueue_->shutdown();
    
}

bool Dispatcher::isShutdown() const {
    return jobQueue_->isShutdown();
}

bool Dispatcher::deliverMessage(const std::shared_ptr<Message>& message) {
    return jobQueue_->pushMessage(message);
}

void Dispatcher::startTaskThreadPool() {
    taskExecutor_->start();
}

}