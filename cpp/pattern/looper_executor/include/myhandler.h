#include "Handler.h"
#pragma once

#define TEST1 1
#define TEST2 2

class myhandler : public Handler {
    public:
        myhandler(std::shared_ptr<SLLooper>& looper)
        : Handler(looper){} 
        void handleMessage(const std::shared_ptr<Message>& msg);
};