#include "Dispatcher.h"
#include "Message.h"
#include<unistd.h>  
#include <sys/time.h>
#include <stdio.h>
#include "Promise.h"
#include <iostream>
#include <string>
using namespace std;




std::shared_ptr<Dispatcher> mExecutor;


class myHandler : public Handler {
    void handleMessage(std::shared_ptr<Message>& msg) {
        std::thread::id id_ = std::this_thread::get_id();
        switch (msg->id) {
            
            default:
                cout<<"[thread ID] ="<<id_<<", task: id="<<msg->id<<", value:"<<msg->payload<<"\n";
                break;
        }
        
        
    }
};

void delayProcess(uint16 mSec) { // delay in mSec seconds
    for (;;) {
        gettimeofday(&tv_current, NULL);
        unsigned long long diff = 
            (tv_current.tv_sec * 1000000 + tv_current.tv_usec) -
            (tv_begin.tv_sec * 1000000 + tv_begin.tv_usec);

        if (diff > mSec*1000000) // 4sec ~ 4000000
            break;
    }
}

int main() {
    std::shared_ptr<Handler> myHandler_ = std::make_shared<myHandler>();
    mExecutor = std::make_shared<Dispatcher>(myHandler_);
    
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



    while (1) {}
    
    return 0;
}