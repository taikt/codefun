#include "Dispatcher.h"
#include "Message.h"
#include<unistd.h>  
#include <sys/time.h>
#include <stdio.h>
#include "Promise.h"
#include "requestMsg.pb.h"

#include <iostream>
#include <string>
using namespace std;
using namespace data;

std::shared_ptr<Dispatcher> disp;
//std::shared_ptr<Promise<int>> mypromise; 

/*
class ResponseMsg {
public:
    int code_;
    string value_;
    int index;
};
*/
struct ResponseMsg {
    int code_;
    string value_;
    int index;
};

std::shared_ptr<Promise<ResponseMsg>> mypromise; 

class myHandler : public Handler {
    void handleMessage(std::shared_ptr<Message>& msg) {
        std::thread::id id_ = std::this_thread::get_id();
        switch (msg->id) {
            case 2:
                
                cout<<"[thread ID] ="<<id_<<", long task start: id="<<msg->id<<", value:"<<msg->payload<<"\n";
                
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

            case 0: {
                sleep(6);
                requestMsg payload;
                payload.ParseFromString(msg->payload);

                ResponseMsg resp;
                resp.code_ = payload.code() + 1;
                resp.value_ = payload.data() + "add more";
                resp.index = 10;
                //cout<<"data:"<<resp.value_<<"\n";
                mypromise->set_value(resp);
                }

                break;

            case 1: {
                requestMsg payload;
                payload.ParseFromString(msg->payload);
                cout<<"[thread ID] ="<<id_<<", msg code="<<payload.code() <<", data="<<payload.data()<<"\n";
                }
                break;

            default:
                cout<<"[thread ID] ="<<id_<<", task: id="<<msg->id<<", value:"<<msg->payload<<"\n";
                break;
        }
        
        
    }
};


Future<ResponseMsg> startRequest() {
	auto promise_obj = std::make_shared<Promise<ResponseMsg>>();
    mypromise = promise_obj;

    std::shared_ptr<Message> msg0 = std::make_shared<Message>(0);

    requestMsg payload;
    payload.set_code(18);
    payload.set_data("hi Tai, are you there!");
    payload.SerializeToString(&msg0->payload);

    //msg0->m_promise =  promise_obj;
    disp->deliverMessage(msg0);
	
	return promise_obj->get_future();
}


int main() {
    std::shared_ptr<Handler> myHandler_ = std::make_shared<myHandler>();

    disp = std::make_shared<Dispatcher>(myHandler_);


    //disp->deliverTask([=]{cout<<"[task] hello Tai\n";});

    startRequest().then(disp, [](const ResponseMsg& msg) {
        cout << "hellu promise:"<<msg.code_<<",index="<<msg.index<<", data:"<<msg.value_<<"\n";
        });

    while (1) {}
    
    return 0;
}