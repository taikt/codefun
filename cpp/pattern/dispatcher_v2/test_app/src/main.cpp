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
//using namespace TPromise;


#define START_REQUEST1 1
#define START_REQUEST2 2
#define START_REQUESTx 3
#define START_REQUEST4 4
#define START_REQUEST5 5

struct ResponseMsg {
    int code_;
    string value_;
    int index;
};

struct ResponseMsg2 {
    int code2_;
    string value2_;
};

std::shared_ptr<Dispatcher> mExecutor;
//std::shared_ptr<Promise<ResponseMsg>> mypromise; 
Promise<ResponseMsg> mypromise; 
std::shared_ptr<Promise<ResponseMsg2>> mypromise2; 
//std::shared_ptr<Promise<int>> mypromise3; 

Promise<int> mypromise3; 

//Promise<double> mypromise3; 

//Future<ResponseMsg2> startRequest2();
//Future<ResponseMsg> startRequest();

class myHandler : public Handler {
    void handleMessage(std::shared_ptr<Message>& msg) {
        std::thread::id id_ = std::this_thread::get_id();
        switch (msg->id) {
            case START_REQUESTx:
                {
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

                requestMsg req;
                req.ParseFromString(msg->payload);

                cout<<"delayed task done: [thread ID] ="<<id_<<", code="<<req.code()<<", data:"<<req.data()<<"\n";
                }
                break;

            case START_REQUEST1: /*{
                
                cout<<"case START_REQUEST1\n";
                sleep(4);
                requestMsg req;
                req.ParseFromString(msg->payload);
                
                ResponseMsg resp;
                resp.code_ = req.code();
                resp.value_ = req.data();
                resp.index = 10;
            
                startRequest2().then(mExecutor, [mypromise, resp](const ResponseMsg2& msg2)
                {
                   ResponseMsg resp2;          
                   resp2.code_ = resp.code_ + msg2.code2_;
                   resp2.value_ = resp.value_ + msg2.value2_;
                   resp2.index = 11;
                   mypromise->set_value(resp2);
                });
                cout<<"START_REQUEST1 end\n";

                }
                */
               cout<<"rec start request1\n";
                break;

            case START_REQUEST2: /*{
                
                cout<<"case START_REQUEST2\n";
                sleep(6);
                requestMsg req;
                req.ParseFromString(msg->payload);
         
                ResponseMsg2 resp;
                resp.code2_ = req.code();
                resp.value2_ = req.data();
                mypromise2->set_value(resp);

                cout<<"START_REQUEST2 end\n";
                }
                */
                cout<<"rec start request2\n";
                break;
            case START_REQUEST4: {
                requestMsg req;
                req.ParseFromString(msg->payload);

                ResponseMsg resp;
                resp.code_ = req.code();
                resp.value_ = req.data();
                resp.index = 10;
                mypromise.set_value(resp);        
            }
            break;

            case START_REQUEST5: {
                requestMsg req;
                req.ParseFromString(msg->payload);

                ResponseMsg resp;
                resp.code_ = req.code();
                resp.value_ = req.data();
          
                cout<<"receive code:"<<resp.code_<<"\n";
                mypromise3.set_value(resp.code_);        
            }
            break;

            default:
                cout<<"[thread ID] ="<<id_<<", task: id="<<msg->id<<", value:"<<msg->payload<<"\n";
                break;
        }
        
        
    }
};

/*
Future<ResponseMsg2> startRequest2() {

    auto promise_obj2 = std::make_shared<Promise<ResponseMsg2>>();
    mypromise2 = promise_obj2;

    std::shared_ptr<Message> msg2 = std::make_shared<Message>(START_REQUEST2);

    requestMsg payload;
    payload.set_code(2);
    payload.set_data("message 2");
    payload.SerializeToString(&msg2->payload);

    mExecutor->deliverMessage(msg2);
	
	return promise_obj2->get_future();
}

Future<ResponseMsg> startRequest() {
	auto promise_obj = std::make_shared<Promise<ResponseMsg>>();
    mypromise = promise_obj;

    std::shared_ptr<Message> msg1 = std::make_shared<Message>(START_REQUEST1);

    requestMsg payload;
    payload.set_code(1);
    payload.set_data("message 1+");
    payload.SerializeToString(&msg1->payload);
    //msg0->m_promise =  promise_obj;
    mExecutor->deliverMessage(msg1);
	
	return promise_obj->get_future();
}
*/

/*
Future<ResponseMsg> startRequest_test() {
	auto promise_obj = std::make_shared<Promise<ResponseMsg>>();
    mypromise = promise_obj;

    std::shared_ptr<Message> msg1 = std::make_shared<Message>(START_REQUEST4);

    requestMsg payload;
    payload.set_code(1);
    payload.set_data("message 1+");
    payload.SerializeToString(&msg1->payload);

    mExecutor->deliverMessage(msg1);
	
	return promise_obj->get_future();
}
*/

Promise<ResponseMsg> startRequest_test() {
	//auto promise_obj = std::make_shared<Promise<ResponseMsg>>();
    auto promise_obj = Promise<ResponseMsg>();

    mypromise = promise_obj;

    std::shared_ptr<Message> msg1 = std::make_shared<Message>(START_REQUEST4);

    requestMsg payload;
    payload.set_code(1);
    payload.set_data("message 1+");
    payload.SerializeToString(&msg1->payload);

    mExecutor->deliverMessage(msg1);
	
	return promise_obj;
}


double startRequest_test2(int& x) {
   double y;
   y = 2.0 + x;
   return y;
}

void sendDelayedMsg() {
    std::shared_ptr<Message> msg = std::make_shared<Message>(START_REQUEST5);

    requestMsg payload;
    payload.set_code(5);
    payload.set_data("message delayed..");
    payload.SerializeToString(&msg->payload);

    mExecutor->deliverMessage(msg);
}

/*
Future<int> doAnotherAsyncCall() {
    //std::shared_ptr<Promise<int>> promise = std::make_shared<Promise<int>>();
    Promise<int> promise;
    mypromise3 = promise;

    sendDelayedMsg();

    return promise.get_future();

}

Future<int> doAsyncall() {

    //std::shared_ptr<Promise<int>> m_promise = std::make_shared<Promise<int>>();
    Promise<int> m_promise;
    // need add mutable!
    // https://stackoverflow.com/questions/27584128/passing-const-cmyclass-as-this-argument-of-discards-qualifiers-fpermis 
    doAnotherAsyncCall().then(mExecutor, [m_promise](int nextResult) mutable{
        std::thread::id id_ = std::this_thread::get_id();

        cout <<"[thread id]="<<id_ <<", next Result:"<<nextResult<<"\n";
        int rep = nextResult + 1;    
        m_promise.set_value(rep);
    });
    

    return m_promise.get_future();
}
*/

int main() {
    std::shared_ptr<Handler> myHandler_ = std::make_shared<myHandler>();
    mExecutor = std::make_shared<Dispatcher>(myHandler_);
    
    //mExecutor->deliverTask([=]{cout<<"[task] hello task\n";});

    startRequest_test().then(mExecutor, [](const ResponseMsg& msg) {
        std::thread::id id_ = std::this_thread::get_id();
        cout <<"[thread id]="<<id_<< ", code="<<msg.code_<<", data: "<<msg.value_<<"\n";
        return 16;
        }).then(mExecutor, [](int x) {
             std::thread::id id_ = std::this_thread::get_id();
            cout <<"[thread id]="<<id_ <<", x="<<x<<"\n";
            return startRequest_test2(x);
        }).then(mExecutor, [](double y) {
            cout <<"sleep 5sec\n";
            sleep(5);
            std::thread::id id_ = std::this_thread::get_id();
            cout <<"[thread id]="<<id_<<", y="<<y<<"\n";
            string str="tai";
            return str;
        }).then(mExecutor, [](string z) {
             std::thread::id id_ = std::this_thread::get_id();
            cout <<"[thread id]="<<id_ <<", z="<<z<<"\n";
        });

    //sendDelayedMsg();

   /*
    doAsyncall().then(mExecutor, [](int mResult){ // or const int& mResult
        cout<<"final result is:"<<mResult<<"\n";
        return 2;
    }).then(mExecutor, [](const int& t) { // note: to check error if int& t, but no error if: int t or const int& t
        cout<<"x="<<t<<"\n";
    });
    */

    cout<<"I am running\n";

    while (1) {}
    
    return 0;
}