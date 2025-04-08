#ifndef MSGTASKTHREAD_HPP
#define MSGTASKTHREAD_HPP
#include <boost/asio/steady_timer.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/bind/bind.hpp>
#include <thread>
#include <map>
#include "JobQueue.h"
#include "Handler.h"

class JobQueue;

class MsgWorkerThread {
 public:
    MsgWorkerThread(std::shared_ptr<JobQueue> taskQueue, std::shared_ptr<Handler> handler);
    ~MsgWorkerThread();
    void start();
    bool isShutdown();


 private:
    // process tasks
    void mainMessageDispatcher();
    void messageDispatcher();
    void handleMessage(std::shared_ptr<Message>& msg);
    void invokeHandler(std::shared_ptr<Message>& msg);
    // weak_ptr for the JobQueue, if task queue is not accessible there is no need to execute
    // tasks
    //std::weak_ptr<JobQueue> jobQueue_;
    std::shared_ptr<JobQueue> jobQueue_;

    // flag to indicate stop execution
    std::atomic_bool shutdown_;
    //int shutdown;

    // Thread to run the tasks on
    std::thread thread_;
    std::shared_ptr<Handler> handler_;
    boost::asio::io_service io_;
    std::shared_ptr<boost::asio::io_service::work> work_;
    std::set<std::shared_ptr<std::thread> > io_threads_;
    

    std::map<std::thread::id, std::shared_ptr<std::thread>> dispatchers_;
};


#endif