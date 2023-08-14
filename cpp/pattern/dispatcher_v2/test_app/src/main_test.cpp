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



void func(int a, int b, int c) {
    std::thread::id id_ = std::this_thread::get_id();
    cout<<"[thread ID] ="<<id_<<", task call:\n";
    cout<<"a="<<a<<", b="<<b<<", c="<<c<<"\n\n";
}

int main() {
    std::shared_ptr<Handler> myHandler_ = std::make_shared<myHandler>();
    mExecutor = std::make_shared<Dispatcher>(myHandler_);
    
    std::thread::id main_id_ = std::this_thread::get_id();

    cout<<"main thread ID ="<<main_id_<<"\n";
    
    
    mExecutor->deliverTask([=]{
        std::thread::id id_ = std::this_thread::get_id();

        cout<<"[thread ID] ="<<id_<<", [task1] hello task...\n";
        });


    uint32_t i = 2;
    mExecutor->deliverTask([i]{
        std::thread::id id_ = std::this_thread::get_id();
        cout<<"[thread ID] ="<<id_<<", [task2] i="<<i<<"\n";
        });

 
    auto fn = std::bind(func,1,3,4);
    mExecutor->deliverTask(fn);


    auto fn2 = std::bind(func,std::placeholders::_1,3,4);
    mExecutor->deliverTask([=]{fn2(9);});
    
	mExecutor->deliverTask([=]{cout<<"hello main\n";});

	

    while (1) {}
    
    return 0;
}