/* 
 * a looper receives messages from different handlers
 * a thread has an unique looper, which has an unique message queue
 *
 * */

#include <stdio.h>
#include <pthread.h>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <semaphore.h>
using namespace std;


#ifndef DEBUG
#define DEBUG 1
#endif


#if DEBUG
#define log(fmt,...) \
    do { \
        char str[100]; \
        sprintf(str,fmt,## __VA_ARGS__); \
        fprintf(stderr, "[%s][%s:%d] %s\n",__FILE__,__func__,__LINE__,str); \
    } while (0)
#else
#define log(...)
#endif

class Handler;
class Handler2;
class Looper;
class MessageQueue;
class Semaphore;
Looper* slooper1 = NULL;
Handler* sHandler1 = NULL;
Handler2* sHandler2 = NULL; // to use directly test(), need declare a derived class



class Semaphore {
    public:
        Semaphore(int value=0);
        ~Semaphore();
        void signal();
        bool wait();

        sem_t mSemaphore;

};

Semaphore::Semaphore(int value) {
    sem_init(&mSemaphore, 0, value);
}

bool Semaphore::wait() {
    return (sem_wait(&mSemaphore) == 0);
}

void Semaphore::signal() {
    sem_post(&mSemaphore);
}

Semaphore::~Semaphore() {
    sem_destroy(&mSemaphore);
}

Semaphore sSemaphore1(0);

class Looper {
    public:
        virtual ~Looper();
        Looper();
        static bool prepare(); 
        //bool prepare(Runnable* runnable);
        static Looper* mylooper(); 
        static void loop(); 
        void quit();
        static void init();
        static void finalize(void* looper);
        MessageQueue& myMessageQueue() {
            return *mMessageQueue;
        }


        //Runnable* runnable;
        MessageQueue* mMessageQueue;
        static pthread_once_t sTlsOneTimeInitializer;
        static pthread_key_t sTlsKey;
};

class Message {
    public:
        virtual ~Message() {}
        Message();
        static Message& obtain(); 
        static Message& obtain(Handler& handler);
        static Message& obtain(Handler& handler, int what);
        //virtual void recycle();
        //void clear();
        //int getExecTimestamp();

        int what;
        //int mExectTimestamp;
        Handler* mHandler;
        Message* mNextMessage;
/*
        friend class MessageQueue;
        friend class Looper;
        friend class Handler;
*/
};

Message::Message():
    what(0),
    mHandler(NULL),
    mNextMessage(NULL)
{
}

Message& Message::obtain() {
    Message* message = new Message();
    return *message;
}

Message& Message::obtain(Handler& handler) {
    Message& message = obtain();
    message.mHandler = &handler;
    return message;
}

Message& Message::obtain(Handler& handler, int what) {
    Message& message = obtain();
    message.mHandler = &handler;
    message.what = what;
    return message;
}

class MessageQueue {
    public:
        MessageQueue();
        ~MessageQueue();
        //bool enqueueMessage(Message& message, int execTimestamp);
        bool enqueueMessage(Message& message);
        Message& dequeueMessage();
        //Message* getNextMessage(int now);
        Message* getNextMessage();

        Message* mHeadMessage;
};


class Handler {
    public:
        Handler();
        ~Handler();
        Handler(Looper& looper);
        Message& obtainMessage() {
            return Message::obtain(*this);
        }
        Message& obtainMessage(int what) {
            return Message::obtain(*this, what);
        }
        void dispatchMessage(Message& message);
        // https://www.geeksforgeeks.org/pure-virtual-functions-and-abstract-classes/
        // https://www.geeksforgeeks.org/virtual-functions-and-runtime-polymorphism-in-c-set-1-introduction/
        // virtual void handleMessage(Message& message); //only declare a virtual function, not define yet
        // virtual void handleMessage(Message& message) {}; // already define vitual function
        // virtual void handleMessage(Message& message)=0; // abstract function or pure virtual function
        virtual void handleMessage(Message& message); //defind it as abstract function (pure virtual function, =0) or need to implement the function in Handler class (base class)
        bool sendMessage(Message& message);

        MessageQueue* mMessageQueue;
};

Handler::Handler(Looper& looper) {
    mMessageQueue = &looper.myMessageQueue();
    log("mMessagequeue=%x", mMessageQueue);
}

Handler::~Handler() {
}

// don't need this definition if the function is abstract
void Handler::handleMessage(Message& message) {
}

Handler::Handler() {
    Looper* looper = Looper::mylooper();
    mMessageQueue = &looper->myMessageQueue();
}



void Handler::dispatchMessage(Message& message) {
    log("message.what=%d",message.what);
    handleMessage(message);
}

bool Handler::sendMessage(Message& message) {
    message.mHandler = this;
    log("mMessageQueue=%x, handler=%x", mMessageQueue,this);
    mMessageQueue->enqueueMessage(message);
}

class Handler1 : public Handler {
    public:
        static const int MSG_PRINT_INFO = 2;
        void handleMessage(Message& message) {
            log("");
            switch (message.what) {
                case MSG_PRINT_INFO:
                    printf("handler1 ID:%x message:%x, iD==%d, looper=%x\n", this, &message, message.what, Looper::mylooper());
                    break;

                default:
                    printf("no valid message\n");
                    break;
            }
        }
};

class Handler2 : public Handler {
    public:
        void handleMessage(Message& message) {
            log("");
            switch (message.what) {
                case 3:
                    printf("handler2 ID:%x message:%x, iD==%d, looper=%x\n", this, &message, message.what, Looper::mylooper());
                    break;

                default:
                    printf("no valid message\n");
                    break;
            }
        }

        void test() {
            sendMessage(obtainMessage(3));
        }
};



MessageQueue::MessageQueue():
    mHeadMessage(NULL)
{
}

MessageQueue::~MessageQueue() {};

bool MessageQueue::enqueueMessage(Message& message) {
    log("");
    Message* curMessage = mHeadMessage;
    if (curMessage == NULL) {
        message.mNextMessage = NULL;
        mHeadMessage = &message;
    } else {
        Message* preMessage = NULL;
        while (curMessage != NULL) {
            preMessage = curMessage;
            curMessage = curMessage->mNextMessage;
        }
        message.mNextMessage = NULL;
        preMessage->mNextMessage = &message;
    }

    return true;
}

Message& MessageQueue::dequeueMessage() {
    while (true) {
        //log("");
        Message* message = getNextMessage();
        if (message != NULL) return *message;
    }
}

Message* MessageQueue::getNextMessage() {
    Message* message = mHeadMessage;
    if (message != NULL) {
        mHeadMessage = message->mNextMessage;
        return message;
    }
}

// need this declaration before using, TODO check static variable in c++
pthread_once_t Looper::sTlsOneTimeInitializer = PTHREAD_ONCE_INIT;
pthread_key_t Looper::sTlsKey;

Looper::Looper() {
    mMessageQueue = new MessageQueue();
    log("mMessageQueue=%x",mMessageQueue);
}

Looper::~Looper() {
    delete mMessageQueue;
}

void Looper::quit() {
    Message& message = Message::obtain();
    mMessageQueue->enqueueMessage(message);
}


void Looper::init() {
    log("");
    pthread_key_create(&sTlsKey, Looper::finalize);
}

void Looper::finalize(void* looper) {
    delete (Looper*) looper;
}

bool Looper::prepare() {
    pthread_once(&sTlsOneTimeInitializer, Looper::init);
    Looper* looper = (Looper*) pthread_getspecific(sTlsKey);
    if (looper == NULL) {
        looper = new Looper();
        pthread_setspecific(sTlsKey, looper);
        log("looper=%x",looper);
    }
    return true;
}

void Looper::loop() {
    Looper* me = mylooper();
    if (me != NULL) {
        MessageQueue* mq = me->mMessageQueue;
        log("");
        while (true) {
            Message& message = mq->dequeueMessage();
            if (message.mHandler == NULL) {
                log("quit looper");
                return;
            }
            log("received a message for handler: %x",message.mHandler);
            message.mHandler->dispatchMessage(message);
        }
    }
}

Looper* Looper::mylooper() {
    Looper* looper = (Looper*) pthread_getspecific(sTlsKey);
    log("looper=%x",looper);
    return looper;
}


class Runnable {
    public:
        virtual ~Runnable() {}
        virtual void Run() = 0;
};

class Thread :public Runnable {
    public:
        Thread(Runnable * runnable);
        Thread();
        virtual ~Thread() {}
        virtual void Run() {}
        void join();
        bool start();

    private:
        static void* exec(void* args);
        Runnable* mRunnable;
        pthread_t mThread;
};


Thread::Thread() :
    mRunnable(this)
{
}

/*
Thread::Thread(Runnable* runnable) :
    mRunnable(runnable) 
{
}
*/

bool Thread::start() {
    log("");
    int error;
    error = pthread_create(&mThread, NULL, &Thread::exec, this);
    return (error == 0);
}

void* Thread::exec(void* args) {
    Thread* const self = (Thread*) args;
    log("");
    self->mRunnable->Run();
    return NULL;
}

void Thread::join() {
    log("");
    pthread_join(mThread,NULL);

}



class Thread1 : public Thread {
    public:
        virtual void Run() {
            log("start looper here\n");
            Looper::prepare();
            slooper1 = Looper::mylooper();
            sHandler1 = new Handler1();
            sHandler2 = new Handler2();
            log("sHandler1=%x",sHandler1);
            sSemaphore1.signal();
            Looper::loop();
        }
};

int main() {
    Thread1 sThread1;
    log("");
    sThread1.start();
    if (sSemaphore1.wait()) {
        Message& msg = sHandler1->obtainMessage(Handler1::MSG_PRINT_INFO);
        log("sHandler1=%x, msg.what=%d",sHandler1,msg.what);
        sHandler1->sendMessage(msg);
        log("sHandler2=%x",sHandler2);
        sHandler2->test();
    }
    
    slooper1->quit();
    sThread1.join();
    
    return 0;
}


