#include "Dispatcher.h"
#include "Message.h"
#include<unistd.h>  
#include <sys/time.h>
#include <stdio.h>
#include "Promise.h"

#include <iostream>
#include <string>
using namespace std;

class myHandler : public Handler {
    void handleMessage(std::shared_ptr<Message>& msg) {
        std::thread::id id_ = std::this_thread::get_id();
        switch (msg->id) {
            case 2:
                
                cout<<"[thread ID] ="<<id_<<", long task start: id="<<msg->id<<", value:"<<msg->str<<"\n";
                
                //sleep(20);
                
                struct timeval tv_begin, tv_current;
                gettimeofday(&tv_begin, NULL);

                for (;;) {
                    gettimeofday(&tv_current, NULL);
                    unsigned long long diff = 
                        (tv_current.tv_sec * 1000000 + tv_current.tv_usec) -
                        (tv_begin.tv_sec * 1000000 + tv_begin.tv_usec);

                    if (diff > 5000000) // 4sec ~ 4000000
                        break;
                }
                
                
                cout<<"[thread ID] ="<<id_<<", task long done\n";
                break;
            case 4:
                sleep(6);
                cout<<"[thread ID] ="<<id_<<", task: id="<<msg->id<<", value:"<<msg->str<<"\n";
                break;

            case 0:
                sleep(10);
                if (msg->m_promise != nullptr) {
                    msg->m_promise->set_value(5);
                }
                break;
            default:
                cout<<"[thread ID] ="<<id_<<", task: id="<<msg->id<<", value:"<<msg->str<<"\n";
                break;
        }
        
        
    }
};


std::shared_ptr<Dispatcher> disp;

//Future<int> startRequest(std::shared_ptr<Dispatcher>& disp) {
Future<int> startRequest() {
	auto promise_obj = std::make_shared<Promise<int>>();

    std::shared_ptr<Message> msg0 = std::make_shared<Message>(0,"abcdasf");
    msg0->m_promise =  promise_obj;
    disp->deliverMessage(msg0);
	
	return promise_obj->get_future();
}

int main() {
    std::shared_ptr<Handler> myHandler_ = std::make_shared<myHandler>();
    //std::shared_ptr<Dispatcher> disp = std::make_shared<Dispatcher>(myHandler_);
    disp = std::make_shared<Dispatcher>(myHandler_);
    std::shared_ptr<Message> msg1 = std::make_shared<Message>(2,"abc");
    std::shared_ptr<Message> msg4 = std::make_shared<Message>(4,"hellu");
    std::shared_ptr<Message> msg2 = std::make_shared<Message>(1,"xyz");
    std::shared_ptr<Message> msg3 = std::make_shared<Message>(5,"xy00z");

    disp->deliverMessage(msg1);
    disp->deliverMessage(msg4);
    disp->deliverMessage(msg2);
    disp->deliverMessage(msg3);

    disp->deliverTask([=]{cout<<"[task] hello Tai\n";});

    startRequest().then(disp, [](int x) {cout << "hellu promise:"<<x<<"\n";});

    while (1) {}
    
    return 0;
}