#include <iostream>
#include <mutex>
#include <thread>
#include "Message.h"
#include <condition_variable>
#pragma once


class MessageQueue
{
public:
    MessageQueue();
   
    bool enqueueMessage(const std::shared_ptr<Message>& message, int64_t whenUs);
    std::shared_ptr<Message> poll();
    void quit();

    bool hasMessage(const std::shared_ptr<Handler>& h, int32_t what, void* obj);


    int64_t uptimeMicros();

private:
    std::shared_ptr<Message> mCurrentMessage;
    std::mutex iMutex;

    bool mStarted;
    mutable bool mQuit;

    std::condition_variable mQueueChanged;

};

