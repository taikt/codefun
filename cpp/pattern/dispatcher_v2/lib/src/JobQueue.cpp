/*
* tai2.tran
*/

#include "JobQueue.h"

JobQueue::JobQueue()
   : shutdown_(false) {
}

std::packaged_task<void()> JobQueue::popTask() {
   
    std::packaged_task<void()> pt;

    std::unique_lock<std::mutex> lock{task_mtx_};
    auto isWait = [this]() { return shutdown_ || !queue_.empty(); };
    if (!isWait()) {
      
        task_cv_.wait(lock, isWait);
    }

    if (!queue_.empty()) {
        pt = std::move(queue_.front());
        queue_.pop_front();
    }
	//cout<<"pop a task\n";
	
    return pt;
}

std::shared_ptr<Message> JobQueue::popMessage() {
    std::shared_ptr<Message> msg;
    std::unique_lock<std::mutex> lock{msg_mtx_};
    auto isWait = [this]() { return shutdown_ || !msg_queue.empty(); };
    if (!isWait()) {

        msg_cv_.wait(lock, isWait);
    }

    if (!msg_queue.empty()) {
        msg = msg_queue.front();
        msg_queue.pop_front();
    }
    return msg;
}

void JobQueue::shutdown() {
    std::lock_guard<std::mutex> lk{msg_mtx_};
    queue_.clear();
    shutdown_ = true;
    msg_cv_.notify_all();
	task_cv_.notify_all();
}

bool JobQueue::isShutdown() {
    return shutdown_;
}

bool JobQueue::pushMessage(const std::shared_ptr<Message>& message) {
    std::lock_guard<std::mutex> lock(msg_mtx_);
    msg_queue.emplace_back(message);
    msg_cv_.notify_one();
}

