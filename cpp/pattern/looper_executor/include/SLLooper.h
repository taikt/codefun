#pragma once

class MessageQueue;

class SLLooper 
{
public:
    SLLooper();
    ~SLLooper();
    std::shared_ptr<MessageQueue> mMessageQueue;
    bool loop();
    std::shared_ptr<MessageQueue> messageQueue();
    void exit(); 

private:
  
    bool mStarted;   
    std::thread t1;
};

