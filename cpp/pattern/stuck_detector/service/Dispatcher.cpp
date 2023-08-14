#include "Log.h"
#include "Dispatcher.hpp"


#define LOG_TAG "Dispatcher"




template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args &&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

Dispatcher::Dispatcher()
   : Dispatcher(1) {
}

Dispatcher::Dispatcher(int threadCount)
   : jobQueue_{std::make_shared<JobQueue>()}
   , threadCount_(threadCount) {

    for (int i = 0; i < threadCount_; i++) {
        auto th = make_unique<WorkerThread>(jobQueue_,i);
        LOGI("start thread %d",i);
        th->start();
        workerThreads_.emplace_back(std::move(th));
    }
}

Dispatcher::~Dispatcher() {

    shutdown();
}

void Dispatcher::shutdown() {

    jobQueue_->shutdown();
    workerThreads_.clear();
}

bool Dispatcher::isShutdown() const {
    return jobQueue_->isShutdown();
}

