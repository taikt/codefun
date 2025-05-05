/*
* tai2.tran
*/

#include "JobQueue.h"


namespace kt {

JobQueue::JobQueue()
   : shutdown_(false) {
}

std::packaged_task<void()> JobQueue::popTask() {
   std::lock_guard<std::mutex> lock(task_mtx_);
    
   std::thread::id id_ = std::this_thread::get_id();
    std::packaged_task<void()> pt;
    //LOGI("[thread:%llu]Jobqueue: pop task: current queue size=%d",id_,queue_.size());

    /*
    std::unique_lock<std::mutex> lock{task_mtx_};
    auto isWait = [this]() { return shutdown_ || !queue_.empty(); };
    if (!isWait()) {
      
        task_cv_.wait(lock, isWait);
    }
    */
    if (!queue_.empty()) {
        //LOGI("Jobqueue: queue not empty");
        pt = std::move(queue_.front());
        queue_.pop_front();
    }

	//LOGI("Jobqueue: return task");
    return pt;
}

std::shared_ptr<Message> JobQueue::popMessage() {
    // to use with condition variable, must use unique_lock instead of lock_guard
    std::unique_lock<std::mutex> lock{msg_mtx_};

    std::shared_ptr<Message> msg;
    
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
    std::lock_guard<std::mutex> lk{task_mtx_}; // TODO: check lock msg too
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

}