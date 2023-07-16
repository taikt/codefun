#include <cstring>
#include "Handler.h"
#include "Message.h"
#include "MessageQueue.h"
#include "SLLooper.h"
#include <iostream>
#include <thread>
#include <string>
using namespace std;

SLLooper::SLLooper()
{
    mMessageQueue = std::make_shared<MessageQueue>();

    mStarted  = false;
    t1 = std::thread(&SLLooper::loop, this);
    cout<<"finish init slloper\n";
}

SLLooper::~SLLooper() {
    mStarted = false;
    t1.join();
}

void SLLooper::exit() {
    mStarted = false;
}

std::shared_ptr<MessageQueue> SLLooper::messageQueue() {
    return mMessageQueue;
}

bool SLLooper::loop()
{
   
    mStarted = true;
    cout<<"start looper\n";

    while (mStarted) {
        cout<<"looper waiting\n";
        std::shared_ptr<Message> message = mMessageQueue->poll();
        cout<<"looper notified msg\n";
        if (message == NULL) {
            return false;
        }
        message->mHandler->dispatchMessage(message);
    }
    mStarted = false;

    cout<<"exit looper\n";

    return false;
}

