/*
* tai2.tran
*/

#include "Dispatcher.h"
#include <iostream>
#include <string>
using namespace std;

/*
template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args &&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}
*/


namespace kt {

Dispatcher::Dispatcher()
   : jobQueue_{std::make_shared<JobQueue>()}
{
    msgExecutor_ = std::make_unique<MsgWorkerThread>(jobQueue_);
    taskExecutor_ = std::make_unique<TaskWorkerThread>(jobQueue_);
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

void Dispatcher::enableTaskConcurrency(bool m_concurrent) {
    taskExecutor_->enableConcurrency(m_concurrent);
}

void Dispatcher::setExpiredTaskTime(uint m_expiredtime) { //ms
    taskExecutor_->setExpiredTime(m_expiredtime);
}

void Dispatcher::setMaxTaskPoolSize(uint m_maxPoolSize) { 
    taskExecutor_->setMaxPoolSize(m_maxPoolSize);
}

void Dispatcher::startTaskThreadPool() {
    taskExecutor_->start();
}

// note: must call setMessageHandler before start message pool
void Dispatcher::setMessageHandler(std::shared_ptr<Handler>& handler) {
    msgExecutor_->setHandler(handler);
}

void Dispatcher::startMessageThreadPool() {
    msgExecutor_->start();
}

}