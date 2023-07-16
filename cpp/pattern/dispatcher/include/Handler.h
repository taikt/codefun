#ifndef HANDLER_HPP
#define HANDLER_HPP

#include "Message.h";

class Handler {
public:
    virtual void handleMessage(std::shared_ptr<Message>& msg) = 0;
};

#endif