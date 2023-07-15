
#include <chrono>

#include "MessageQueue.h"
#include <iostream>
using namespace std;



namespace sl {
MessageQueue::MessageQueue() :
        mCurrentMessage(NULL),
        mStarted(false), mQuit(false)
{
}

MessageQueue::~MessageQueue()
{
}

bool MessageQueue::enqueueMessage(const std::shared_ptr<Message>& message, int64_t whenUs)
{
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

bool MessageQueue::removeMessages(const std::shared_ptr<Handler>& h, int32_t what, void* obj)
{
    if (h == NULL) {
        return false;
    }

    std::lock_guard<std::mutex> m_lock(iMutex);
    std::shared_ptr<Message> cur = mCurrentMessage;
    bool sendSignal = false;
    while (cur != NULL &&
           cur->mHandler == h && cur->what == what
           && (obj == NULL || cur->obj == obj)) {
#if defined(DBG_MESSAGE_QUEUE_REMOVE)
        LOGI("removeMessages : Found messsage what:%d[%d] next", cur->what, what);
#endif // DBG_MESSAGE_QUEUE

        mCurrentMessage = cur->mNextMessage;
        sendSignal = true;
        //cur->recycle();
        cur = mCurrentMessage;
    }

    // Remove all messages after front.
    while (cur != NULL) {
        std::shared_ptr<Message> next = cur->mNextMessage;
        if (next != NULL) {
            if (next->mHandler == h && next->what == what
                && (obj == NULL || next->obj == obj)) {
#if defined(DBG_MESSAGE_QUEUE_REMOVE)
                LOGI("removeMessages : Found messsage from front what:%d[%d]", next->what, what);
#endif // DBG_MESSAGE_QUEUE
                std::shared_ptr<Message> nn = next->mNextMessage;
                //next->recycle();
                cur->mNextMessage = nn;
                sendSignal = true;
                continue;
            }
        }
        cur = next;
    }

    if (sendSignal && mStarted) {
        mQueueChanged.notify_one();
    }
    return sendSignal;
}

std::shared_ptr<Message> MessageQueue::poll()
{
    while (!mQuit) {

#if 0
#if defined(DBG_MESSAGE_QUEUE)
        LOGI("<<<  poll start : 0x%x", this);
        android::Mutex::Autolock _l(mLock);
        LOGI("poll started >>>: 0x%x", this);
#else
        android::Mutex::Autolock _l(mLock);
#endif // DBG_MESSAGE_QUEUE
#endif

        mStarted = true;
        if (mCurrentMessage == NULL) {
            std::unique_lock lk(iMutex);
            mQueueChanged.wait(lk, [this]{return (mCurrentMessage != nullptr);});
        }

        if (mCurrentMessage == NULL) {
            LOGI("wakeup from wait but messaage is NULL");
            continue;
        }

#if 0

        int64_t nowUs = uptimeMicros();
        int64_t whenUs = mCurrentMessage->whenUs;
#if defined(DBG_MESSAGE_QUEUE)
        LOGI("now:%lld when:%lld", nowUs, whenUs);
#endif // DBG_MESSAGE_QUEUE

        if (whenUs > nowUs) {
            int64_t delayUs = whenUs - nowUs;
#if defined(DBG_MESSAGE_QUEUE)
            LOGI("start waitRelative [0x%x]  0x%x", this, mCurrentMessage.get());
#endif // DBG_MESSAGE_QUEUE
            mQueueChanged.waitRelative(mLock, us2ns(delayUs));

/*
#if defined(DBG_MESSAGE_QUEUE)
        LOGI("wakeup from  waitRelative now:%lld when:%lld", ns2us(systemTime(SYSTEM_TIME_MONOTONIC)), whenUs);
#endif // DBG_MESSAGE_QUEUE
            //continue;
        }

        if(mCurrentMessage == NULL)
        {
            LOGI("wakeup from waitRelative but messaage is NULL");
*/
            continue;
        }
#if defined(DBG_MESSAGE_QUEUE)
        LOGI("<<>><<>>Got Message[0x%x]  0x%x", this, mCurrentMessage.get());
#endif // DBG_MESSAGE_QUEUE

#endif
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
}  //namespace sl
