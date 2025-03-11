#include "JobQueue.h"

JobQueue::JobQueue()
   : shutdown_(false) {
}

std::packaged_task<void()> JobQueue::popTask() {
    std::packaged_task<void()> pt;

    std::unique_lock<std::mutex> lock{mtx_};
    auto isWait = [this]() { return shutdown_ || !queue_.empty(); };
    if (!isWait()) {
        //printf("JobQueue: waiting for task");
        cv_.wait(lock, isWait);
    }
    //printf("JobQueue: task is available, get task now");
    if (!queue_.empty()) {
        pt = std::move(queue_.front());
        queue_.pop_front();
    }
    return pt;
}

void JobQueue::shutdown() {
    std::lock_guard<std::mutex> lk{mtx_};
    queue_.clear();
    shutdown_ = true;
    cv_.notify_all();
}

bool JobQueue::isShutdown() {
    return shutdown_;
}

