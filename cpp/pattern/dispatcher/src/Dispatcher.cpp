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


Dispatcher::Dispatcher(std::shared_ptr<Handler>& handler)
   : jobQueue_{std::make_shared<JobQueue>()}
   , handler_(handler) {
    /*
    for (int i = 0; i < threadCount_; i++) {
        auto th = make_unique<WorkerThread>(jobQueue_,i);
        //LOGI("start thread %d",i);
        th->start();
        workerThreads_.emplace_back(std::move(th));
    }
    */
    auto th1 = std::make_unique<MsgWorkerThread>(jobQueue_,handler_);
    th1->start();
    MsgworkerThreads_.emplace_back(std::move(th1));
    auto th2 = std::make_unique<TaskWorkerThread>(jobQueue_);
    th2->start();
    TaskworkerThreads_.emplace_back(std::move(th2));
}

Dispatcher::~Dispatcher() {
    shutdown();
}

void Dispatcher::shutdown() {
    cout<<"dispatcher shutdown\n";
    jobQueue_->shutdown();
    MsgworkerThreads_.clear();
    TaskworkerThreads_.clear();
}

bool Dispatcher::isShutdown() const {
    return jobQueue_->isShutdown();
}

bool Dispatcher::deliverMessage(const std::shared_ptr<Message>& message) {
    return jobQueue_->pushMessage(message);
}

