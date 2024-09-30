#include "Dispatcher.h"
using namespace std;

template <typename T, typename... Args>
std::unique_ptr<T> make_unique(Args &&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

Dispatcher::Dispatcher()
   : Dispatcher(3,4) {
}

Dispatcher::Dispatcher(int inThreadCount, int outThreadCount)
   : inQueue_{std::make_shared<JobQueue>()},
    outQueue_{std::make_shared<JobQueue>() },
   inThreadCount_(inThreadCount),
   outThreadCount_(outThreadCount) {

    for (int i = 0; i < inThreadCount_; i++) {
        auto th = std::make_unique<WorkerThread>(inQueue_,i);
        printf("start inThread %d\n",i);
        th->start();
        inWorkerThreads_.emplace_back(std::move(th));
    }

    for (int i = 0; i < outThreadCount_; i++) {
        auto th = std::make_unique<WorkerThread>(outQueue_, i);
        printf("start outThread %d\n", i);
        th->start();
        outWorkerThreads_.emplace_back(std::move(th));
    }
}

Dispatcher::~Dispatcher() {

    shutdown();
}

void Dispatcher::shutdown() {

    inQueue_->shutdown();
    inWorkerThreads_.clear();
    outQueue_->shutdown();
    outWorkerThreads_.clear();
}

bool Dispatcher::isShutdown() const {
    return (inQueue_->isShutdown() && outQueue_->isShutdown());
}

