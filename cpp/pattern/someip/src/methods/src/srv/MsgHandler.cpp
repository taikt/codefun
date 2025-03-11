#include "MsgHandler.h"

Promise<ResponseMsg> mypromise;

void MsgHandler::handleMessage(std::shared_ptr<Message>& msg){
    std::thread::id id_ = std::this_thread::get_id();
    switch (msg->id) {
        case START_REQUEST1:
            {      
            }
            break;

        default:
            cout<<"[thread ID] ="<<id_<<", task: id="<<msg->id<<", value:"<<msg->payload<<"\n";
            break;
    }  
    
};