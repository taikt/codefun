
namespace sl {
class Handler;

/**
 * This is Container data and object which can be send to Handler via MessageSender or
 * enqueue into MessageQueue directly.
 * You can choice the way you want.
 *
 * Message support converting to Parcel so that Message can be posted via IPC as binder
 * so that remote instance can get same Message and take information .
 *
 * Message Ctor is public but you can use Message::obtain() to create.
 *
 * Currently, Service layer is providing two kind of Message.
 * One is this and the other is android::Message.
 * android::Message only can have one int command and ::Message can have various type of
 * message such as what, cmd1, cmd2, cmd3 and parcel.
 */

class RefBase 
{
public:
	int id;
	std::string method;
}

class Message 
{
public:
    Message();
    Message(Message& other);
    virtual ~Message();

    static std::shared_ptr<Message> obtain();
    static std::shared_ptr<Message> obtain(const Message* message);
    static std::shared_ptr<Message> obtain(const std::shared_ptr<Handler>& handler);
    static std::shared_ptr<Message> obtain(const std::shared_ptr<Handler>& handler, int32_t obtain_what);
    static std::shared_ptr<Message> obtain(const std::shared_ptr<Handler>& handler, int32_t obtain_what,
                                 int32_t obtain_arg1);
    static std::shared_ptr<Message> obtain(const std::shared_ptr<Handler>& handler, int32_t obtain_what,
                                 void* obtain_obj);
    static std::shared_ptr<Message> obtain(const std::shared_ptr<Handler>& handler, int32_t obtain_what,
                                 int32_t obtain_arg1, int32_t obtain_arg2);
    static std::shared_ptr<Message> obtain(const std::shared_ptr<Handler>& handler, int32_t obtain_what,
                                 int32_t obtain_arg1, int32_t obtain_arg2, void* obtain_obj);
    static std::shared_ptr<Message> obtain(const std::shared_ptr<Handler>& handler, int32_t obtain_what,
                                 int32_t obtain_arg1, int32_t obtain_arg2, int32_t obtain_arg3);
	
    static std::shared_ptr<Message> obtain(const std::shared_ptr<Handler>& handler, int32_t obtain_what,
                                 std::shared_ptr<RefBase> obtain_spRef);
	
	/*
	template<typename T>
	static std::shared_ptr<Message> obtain(const std::shared_ptr<Handler>& handler, int32_t obtain_what,
                                 std::shared_ptr<T> obtain_spRef);
	*/
    bool sendToTarget();

    std::shared_ptr<Message> dup() const;

    template<typename T>
    void getObject(std::shared_ptr<T>& obj)
    {
        obj = static_cast<T*>( spRef.get() );
    }


    void dump();

protected:
    Message& operator=(const Message& other);
    void clear();

private:
    void setTo(const Message& other);
    std::shared_ptr<Handler> mHandler;
    std::shared_ptr<Message> mNextMessage;

public:
    int32_t what;
    int32_t arg1;
    int32_t arg2;
    int32_t arg3;

    void* obj;
    ssize_t obj_size;

    Buffer buffer;
    Buffer buffer2;
    std::shared_ptr<Post> post;
    std::shared_ptr<RefBase> spRef;

private:
    int64_t whenUs;

    friend class Handler;
    friend class MessageQueue;
    friend class SLLooper;
};
} //namespace sl
#endif // SERVICE_LAYER_UTILS_MESSAGE_H