#include <iostream>
#include <mutex>
#include <thread>
#include <condition_variable>

namespace sl {
class MessageQueue
{
public:
    MessageQueue();
    virtual ~MessageQueue();
    bool enqueueMessage(const std::shared_ptr<Message>& message, int64_t whenUs);
    std::shared_ptr<Message> poll();
    void quit();

    bool hasMessage(const std::shared_ptr<Handler>& h, int32_t what, void* obj);
    bool removeMessages(const std::shared_ptr<Handler>& h, int32_t what, void* obj);

    int64_t uptimeMicros();

private:
    std::shared_ptr<Message> mCurrentMessage;
    std::mutex iMutex;

    bool mStarted;
    mutable bool mQuit;

    std::condition_variable mQueueChanged;

};
} //namespace sl
#endif // SERVICE_LAYER_UTILS_MESSAGE_QUEUE_H