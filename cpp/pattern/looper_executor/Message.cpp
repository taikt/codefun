include <cassert>


#include "Message.h"
#include "Handler.h"
#include "Buffer.h"


namespace sl {
Message::Message() :
    what(0),
    arg1(0),
    arg2(0),
    arg3(0),
    obj(NULL),
    obj_size(0),
    whenUs(0)
{
    mNextMessage = NULL;
    mHandler = NULL;
    post = NULL;
    spRef = NULL;
}

Message::Message(Message& other) :
    what(0),
    arg1(0),
    arg2(0),
    arg3(0),
    obj(NULL),
    obj_size(0),
    whenUs(0),
    spRef(NULL)
{
    setTo(other);
}

Message::~Message()
{
}

std::shared_ptr<Message> Message::obtain()
{
	auto msg = std::make_shared<Message>();
	return msg;
}

std::shared_ptr<Message> Message::obtain(const std::shared_ptr<Handler>& handler)
{
    std::shared_ptr<Message> message = obtain();
    message->mHandler = handler;
    return message;
}

std::shared_ptr<Message> Message::obtain(const std::shared_ptr<Handler>& handler, int32_t obtain_what)
{
    std::shared_ptr<Message> message = obtain();
    message->mHandler = handler;
    message->what = obtain_what;
    return message;
}

std::shared_ptr<Message> Message::obtain(const std::shared_ptr<Handler>& handler, int32_t obtain_what
                               , int32_t obtain_arg1)
{
    std::shared_ptr<Message> message = obtain();
    message->mHandler = handler;
    message->what = obtain_what;
    message->arg1 = obtain_arg1;
    return message;
}

std::shared_ptr<Message> Message::obtain(const std::shared_ptr<Handler>& handler, int32_t obtain_what
                               , void* obtain_obj)
{
    std::shared_ptr<Message> message = obtain();
    message->mHandler = handler;
    message->what = obtain_what;
    message->obj = obtain_obj;
    return message;
}


std::shared_ptr<Message> Message::obtain(const std::shared_ptr<Handler>& handler, int32_t obtain_what
                               , int32_t obtain_arg1, int32_t obtain_arg2)
{
    std::shared_ptr<Message> message = obtain();
    message->mHandler = handler;
    message->what = obtain_what;
    message->arg1 = obtain_arg1;
    message->arg2 = obtain_arg2;
    return message;
}


std::shared_ptr<Message> Message::obtain(const std::shared_ptr<Handler>& handler, int32_t obtain_what
                               , int32_t obtain_arg1, int32_t obtain_arg2, void* obtain_obj)
{
    std::shared_ptr<Message> message = obtain();
    message->mHandler = handler;
    message->what = obtain_what;
    message->arg1 = obtain_arg1;
    message->arg2 = obtain_arg2;
    message->obj = obtain_obj;
    return message;
}


std::shared_ptr<Message> Message::obtain(const std::shared_ptr<Handler>& handler, int32_t obtain_what
                               , int32_t obtain_arg1, int32_t obtain_arg2, int32_t obtain_arg3)
{
    std::shared_ptr<Message> message = obtain();
    message->mHandler = handler;
    message->what = obtain_what;
    message->arg1 = obtain_arg1;
    message->arg2 = obtain_arg2;
    message->arg3 = obtain_arg3;
    return message;
}

std::shared_ptr<Message> Message::obtain(const Message* message)
{
    std::shared_ptr<Message> dupMessage = obtain();
    dupMessage->what = message->what;
    dupMessage->arg1 = message->arg1;
    dupMessage->arg2 = message->arg2;
    dupMessage->arg3 = message->arg3;
    dupMessage->obj = message->obj;
    dupMessage->spRef = message->spRef;
    dupMessage->whenUs = 0;
    dupMessage->mHandler = message->mHandler;
    dupMessage->mNextMessage = NULL;
    return dupMessage;
}


std::shared_ptr<Message> Message::obtain(const std::shared_ptr<Handler>& handler, int32_t obtain_what
                               , std::shared_ptr<RefBase> obtain_spRef)
{
    std::shared_ptr<Message> message = obtain();
    message->mHandler = handler;
    message->what = obtain_what;
    message->spRef= obtain_spRef;
    return message;
}




bool Message::sendToTarget()
{
    if (mHandler != NULL) {
        return mHandler->sendMessage(this);
    } else {
        cout<<"sendToTarget handler is NULL\n";
        return false;
    }
}

std::shared_ptr<Message> Message::dup() const
{
    return Message::obtain(this);
}

void Message::clear()
{
    what = 0;
    arg1 = 0;
    arg2 = 0;
    arg3 = 0;
    obj = NULL;
    obj_size = 0;
    whenUs = 0;
    mNextMessage = NULL;
    mHandler = NULL;
    post = NULL;
    spRef = NULL;
}


Message& Message::operator=(const Message& other)
{
    setTo(other);
    return *this;
}

void Message::setTo(const Message& other)
{
    if(this != &other)
    {
        what = other.what;
        arg1 = other.arg1;
        arg2 = other.arg2;
        arg3 = other.arg3;
        obj = other.obj;
        obj_size = other.obj_size;
        whenUs = other.whenUs;
        post = other.post;
        spRef = other.spRef;
    }
}

#if 0
void Message::dump()
{
    char buf[1024] = {0, };
    snprintf(buf,sizeof(buf), "dump Message what:%d, arg1:%d arg2:%d arg3:%d whenUs:%lld\n",
        what, arg1, arg2, arg3, whenUs);
    LOGI("%s", buf);
    if(obj != NULL)
    {
        std::shared_ptr<Buffer> dump_buf = new Buffer();
        dump_buf->setTo((uint8_t*)obj, obj_size);
        dump_buf->dump();
    }
}


error_t Message::writeToParcel(android::Parcel* parcel)
{
    parcel->writeInt32(what);
    parcel->writeInt32(arg1);
    parcel->writeInt32(arg2);
    parcel->writeInt32(arg3);

    parcel->writeInt32(obj_size);
    if(obj_size != 0)
        parcel->write(obj, static_cast<uint32_t>(obj_size));

    parcel->writeInt32(static_cast<int32_t>(buffer.size()));
    if(buffer.size() != 0U)
        parcel->write(buffer.data(), buffer.size());

    parcel->writeInt32(static_cast<int32_t>(buffer2.size()));
    if(buffer2.size() != 0U)
        parcel->write(buffer2.data(), buffer2.size());

    post->writeToParcel(parcel);
    return E_OK;
}

error_t Message::readFromParcel(android::Parcel& parcel)
{
    what = parcel.readInt32();
    arg1 = parcel.readInt32();
    arg2 = parcel.readInt32();
    arg3 = parcel.readInt32();

    obj_size = parcel.readInt32();
    if(obj_size != 0) {
        parcel.read(obj, static_cast<uint32_t>(obj_size));
    }

    buffer.setSize(parcel.readInt32());
    if(buffer.size() != 0U) {
        parcel.read(buffer.data(), buffer.size());
    }

    buffer2.setSize(parcel.readInt32());
    if(buffer2.size() != 0U) {
        parcel.read(buffer2.data(), buffer2.size());
    }

    if(post == NULL) {
        post = new Post();
    }
    post->readFromParcel(parcel);

    return E_OK;
}
#endif

}// namespace sl
