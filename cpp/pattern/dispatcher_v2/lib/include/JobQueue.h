 #ifndef TASKQUEUE_HPP
#define TASKQUEUE_HPP

#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <deque>
#include "Message.h"

class JobQueue {
 public:
    JobQueue();

    template <typename F, typename... Args>
    auto pushTask(F task, Args &&... args) -> std::future<decltype(task(args...))> {
        return pushTaskTo(std::forward<F>(task), std::forward<Args>(args)...);
    }

    bool pushMessage(const std::shared_ptr<Message>& message);

    std::packaged_task<void()> popTask();
    std::shared_ptr<Message> popMessage();

    void shutdown();
    bool isShutdown();

    int getMsgQueueSize() {
        return msg_queue.size();
    }

 private:
    // queue of tasks
    std::deque<std::packaged_task<void()>> queue_;

    // queue of messages
    std::deque<std::shared_ptr<Message>> msg_queue;

    std::mutex mtx_;
    std::condition_variable cv_;

    std::atomic_bool shutdown_;

    template <typename F, typename... Args>
    auto pushTaskTo(F task, Args &&... args) -> std::future<decltype(task(args...))>;

    
};

template <typename F, typename... Args>
auto JobQueue::pushTaskTo(F task, Args &&... args) -> std::future<decltype(task(args...))> {

    using returnType = decltype(task(args...));
    auto boundedTask = std::bind(std::forward<F>(task), std::forward<Args>(args)...);

    auto pkgedTask = std::packaged_task<returnType()>(boundedTask);
    auto future = pkgedTask.get_future();
    {
        std::lock_guard<std::mutex> lock(mtx_);
        queue_.emplace_back(std::move(pkgedTask));
    }
    cv_.notify_one();

    return future;
}



#endif