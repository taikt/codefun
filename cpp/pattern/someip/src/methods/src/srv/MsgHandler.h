#ifndef MYHANDLER_H
#define MYHANDLER_H
#include "Promise.h"
#include "Dispatcher.h"
#include "Message.h"
#include <v1/taikt/myInterface.hpp>


using namespace v1_2::taikt;
using namespace kt;

#define START_REQUEST1 1

struct ResponseMsg {
    int32_t y1;
    std::string y2;
    myInterface::stdErrorTypeEnum methodError;
};
class MsgHandler : public Handler {
public:
 
    void handleMessage(std::shared_ptr<Message>& msg);
};



#endif