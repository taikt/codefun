/*
* tai2.tran
*/

#include "JobQueue.h"


namespace kt {

JobQueue::JobQueue()
   : shutdown_(false) {
}

std::packaged_task<void()> JobQueue::popTask() {
   std::unique_lock<std::mutex> lock(task_mtx_);
    
   std::thread::id id_ = std::this_thread::get_id();
    std::packaged_task<void()> pt;
    //LOGI("[thread:%llu]Jobqueue: pop task: current queue size=%d",id_,queue_.size());

    auto isWait = [this]() { return shutdown_ || !queue_.empty(); };
    if (!isWait()) {
      
        task_cv_.wait(lock, isWait);
    }
    
    if (!queue_.empty()) {
        //LOGI("Jobqueue: queue not empty");
        pt = std::move(queue_.front());
        queue_.pop_front();
    }

	//LOGI("Jobqueue: return task");
    return pt;
}


void JobQueue::shutdown() {
    std::lock_guard<std::mutex> lk{task_mtx_}; // TODO: check lock msg too
    queue_.clear();
    shutdown_ = true;
  
	task_cv_.notify_all();
}

bool JobQueue::isShutdown() {
    return shutdown_;
}

}