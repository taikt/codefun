//#include "Log.h"
#include "JobQueue.h"

JobQueue::JobQueue()
   : shutdown_(false) {
}

std::packaged_task<void()> JobQueue::popTask() {
    ////LOGI("JobQueue: request to popTask task");
    std::packaged_task<void()> pt;

    std::unique_lock<std::mutex> lock{mtx_};
    auto isWait = [this]() { return shutdown_ || !queue_.empty(); };
    if (!isWait()) {
        ////LOGI("JobQueue: waiting for task");
        cv_.wait(lock, isWait);
    }
    ////LOGI("JobQueue: task is available, get task now");
    if (!queue_.empty()) {
        pt = std::move(queue_.front());
        queue_.pop_front();
    }
    return pt;
}

std::shared_ptr<Message> JobQueue::popMessage() {
    std::shared_ptr<Message> msg;
    std::unique_lock<std::mutex> lock{mtx_};
    auto isWait = [this]() { return shutdown_ || !msg_queue.empty(); };
    if (!isWait()) {
        ////LOGI("JobQueue: waiting for task");
        cv_.wait(lock, isWait);
    }
    ////LOGI("JobQueue: task is available, get task now");
    if (!msg_queue.empty()) {
        msg = msg_queue.front();
        msg_queue.pop_front();
    }
    return msg;
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

bool JobQueue::pushMessage(const std::shared_ptr<Message>& message) {
    std::lock_guard<std::mutex> lock(mtx_);
    msg_queue.emplace_back(message);
    cv_.notify_one();
}

