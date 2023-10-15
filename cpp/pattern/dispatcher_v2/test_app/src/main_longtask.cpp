#include "Dispatcher.h"
#include "Message.h"
#include<unistd.h>  
#include <sys/time.h>
#include <stdio.h>
#include "Promise.h"
#include <iostream>
#include <string>
using namespace std;
using namespace kt;



std::shared_ptr<Dispatcher> mExecutor;


void delayProcess(uint mSec) { // delay in mSec seconds
    struct timeval tv_begin, tv_current;
    gettimeofday(&tv_begin, NULL);

    for (;;) {
        gettimeofday(&tv_current, NULL);
        unsigned long long diff = 
            (tv_current.tv_sec * 1000000 + tv_current.tv_usec) -
            (tv_begin.tv_sec * 1000000 + tv_begin.tv_usec);

        if (diff > mSec*1000000) // 4sec ~ 4000000
            break;
    }
}


void doSomething() {
}

int main() {

	// initiate task executor
    mExecutor = std::make_shared<Dispatcher>();
	
	// enable concurrency
    mExecutor->enableTaskConcurrency(true);

	// set size of thread pool
    mExecutor->setMaxTaskPoolSize(15);

	// start thread pool
    mExecutor->startTaskThreadPool();





	// run task asynchronously
	mExecutor->deliverTask([=]{doSomething();});

	// delay 10s before running task
	mExecutor->deliverTask([=]{10s, doSomething();});

	
    
    std::thread::id main_id_ = std::this_thread::get_id();

    cout<<"main thread ID ="<<main_id_<<"\n";
    
    
    mExecutor->deliverTask([=]{
        std::thread::id id_ = std::this_thread::get_id();
        delayProcess(3);
        cout<<"[thread ID] ="<<id_<<", [task1] hello task...\n";
        });


    uint32_t i = 2;
    mExecutor->deliverTask([i]{
        std::thread::id id_ = std::this_thread::get_id();
        delayProcess(5);
        cout<<"[thread ID] ="<<id_<<", [task2] i="<<i<<"\n";
        });

    mExecutor->deliverTask([=]{
        std::thread::id id_ = std::this_thread::get_id();
        delayProcess(4);
        cout<<"[thread ID] ="<<id_<<", [task3] hello task3\n";
        });

    mExecutor->deliverTask([=]{
        std::thread::id id_ = std::this_thread::get_id();
        delayProcess(0.5);
        cout<<"[thread ID] ="<<id_<<", [task4] hello task4\n";
        });

    mExecutor->deliverTask([=]{
        std::thread::id id_ = std::this_thread::get_id();
        delayProcess(7);
        cout<<"[thread ID] ="<<id_<<", [task5] hello tash5\n";
        });
    
    mExecutor->deliverTask([=]{
        std::thread::id id_ = std::this_thread::get_id();
        delayProcess(2);
        cout<<"[thread ID] ="<<id_<<", [task6] hello task6\n";
        });
    mExecutor->deliverTask([=]{
        std::thread::id id_ = std::this_thread::get_id();
        delayProcess(1);
        cout<<"[thread ID] ="<<id_<<", [task7] hello task7\n";
        });

    delayProcess(2);

    mExecutor->deliverTask([=]{
        std::thread::id id_ = std::this_thread::get_id();
        delayProcess(1);
        cout<<"[thread ID] ="<<id_<<", [task8] hello task8\n";
        });
    mExecutor->deliverTask([=]{
        std::thread::id id_ = std::this_thread::get_id();
        delayProcess(2);
        cout<<"[thread ID] ="<<id_<<", [task9] hello task9\n";
        });

    delayProcess(10);

    mExecutor->deliverTask([=]{
        std::thread::id id_ = std::this_thread::get_id();
        delayProcess(4);
        cout<<"[thread ID] ="<<id_<<", [task10] hello task10\n";
        });
    mExecutor->deliverTask([=]{
        std::thread::id id_ = std::this_thread::get_id();
        delayProcess(2);
        cout<<"[thread ID] ="<<id_<<", [task11] hello task11\n";
        });


    while (1) {}
    
    return 0;
}