#include <chrono>
#include "Message.h"
#include "MessageQueue.h"
#include <mutex>
#include <thread>
#include <iostream>
using namespace std;
#include <string>


MessageQueue::MessageQueue() :
        mCurrentMessage(NULL),
        mStarted(false), mQuit(false)
{
}

bool MessageQueue::enqueueMessage(const std::shared_ptr<Message>& message, int64_t whenUs)
{
    cout<<"message queue: receive msg\n";
    bool sendSignal = false;

	std::lock_guard<std::mutex> m_lock(iMutex);
    message->whenUs = whenUs;
    if (mCurrentMessage == NULL || whenUs < mCurrentMessage->whenUs) {
        message->mNextMessage = mCurrentMessage;
        mCurrentMessage = message;
        sendSignal = true;
    } else {
        std::shared_ptr<Message> prev = NULL;
        std::shared_ptr<Message> cur = mCurrentMessage;
        while (cur != NULL && cur->whenUs <= whenUs) {
            prev = cur;
            cur = cur->mNextMessage;
        }
        message->mNextMessage = prev->mNextMessage;
        prev->mNextMessage = message;
        sendSignal = true;
    }

    if (sendSignal && mStarted) {
        mQueueChanged.notify_one();
    }
    return true;
}

bool MessageQueue::hasMessage(const std::shared_ptr<Handler>& h, int32_t what, void* obj)
{
    if (h == NULL) {
        return false;
    }
    std::lock_guard<std::mutex> m_lock(iMutex);
    std::shared_ptr<Message> cur = mCurrentMessage;
    while (cur != NULL &&
           cur->mHandler == h && cur->what == what
           && (obj == NULL || cur->obj == obj))
    {
#if defined(DBG_MESSAGE_QUEUE_REMOVE)
        LOGI("hasMessage : Found messsage what:%d next", cur->what);
#endif // DBG_MESSAGE_QUEUE
        return true;
    }

    // Remove all messages after front.
    while (cur != NULL) {
        std::shared_ptr<Message> next = cur->mNextMessage;
        if (next != NULL) {
            if (next->mHandler == h && next->what == what
                && (obj == NULL || next->obj == obj)) {
#if defined(DBG_MESSAGE_QUEUE_REMOVE)
                LOGI("hasMessage : Found messsage from front what:%d", next->what);
#endif // DBG_MESSAGE_QUEUE
                return true;
            }
        }
        cur = next;
    }

    return false;
}

std::shared_ptr<Message> MessageQueue::poll()
{
    while (!mQuit) {

        mStarted = true;
        if (mCurrentMessage == NULL) {
            std::unique_lock<std::mutex> lk(iMutex);
            mQueueChanged.wait(lk, [this]{return (mCurrentMessage != nullptr);});
        }

        std::shared_ptr<Message> message = mCurrentMessage;
        mCurrentMessage = message->mNextMessage;

        return message;
    }

    return NULL;
}

void MessageQueue::quit()
{
    mQuit = true;
    mQueueChanged.notify_one();
}

int64_t MessageQueue::uptimeMicros()
{
    auto now = std::chrono::steady_clock::now();
    auto dur = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch());
    return dur.count();
}

