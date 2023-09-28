/*
* tai2.tran
*/

#ifndef HANDLER_HPP
#define HANDLER_HPP


#include "Message.h";

namespace kt {
class Handler {
public:
    //virtual void handleMessage(std::shared_ptr<Message>& msg) = 0;
    void handleMessage(std::shared_ptr<Message>& msg){};
};

} // end kt

#endif