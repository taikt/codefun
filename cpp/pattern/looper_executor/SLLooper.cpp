
#include <cutils/process_name.h>
#include <cstring>

#include "Log.h"
#include "utils/Handler.h"
#include "utils/Message.h"
#include "utils/MessageQueue.h"
#include "utils/SLLooper.h"

#undef LOG_TAG
#define LOG_TAG "SLLooper"

#ifdef Thread_Watcher_Handler
#include "utils/THandler.h"
#endif

using android::OK;

static pthread_once_t gTLSOnce = PTHREAD_ONCE_INIT;
static pthread_key_t gTLSKey = 0;

namespace sl {
const char* SLLooper::SERVICE_PREFIX = "service_layer";
const size_t SLLooper::SERVICE_PREFIX_SIZE = 13;

/**
*   SLLooper()
*   Ctor of SLLooper
*
* \param  NONE
* \return NONE
* \see    android::Looper, MessageQueue
*/
#ifdef Thread_Watcher_Handler
SLLooper::SLLooper(const int reportPeriodSec) :
    mReportSec(reportPeriodSec)
{
    mMessageQueue = NULL;
    mLooperThread = NULL;
    mLooperName = new Buffer();
    mStarted  = false;
}
#else
SLLooper::SLLooper()
{
    mMessageQueue = NULL;
    mLooperThread = NULL;
    mLooperName = new Buffer();
    mStarted  = false;
}
#endif

/**
*   ~SLLooper()
*   Dtor of SLLooper
*
* \param  NONE
* \return NONE
* \see    android::Looper, MessageQueue
*/
SLLooper::~SLLooper()
{
    mMessageQueue = NULL;
    mLooperThread = NULL;
    mLooperName = NULL;
    mStarted  = false;
}

/**
*     prepare
*     prepare Looper instance and assign into current thread if possible.
*
* \param  NONE
* \return boot true if success, else false
* \see    android::Looper, MessageQueue
*/
bool SLLooper::prepare()
{
    SLLooper::setForThread(this);

    if (mMessageQueue == NULL) {
        mMessageQueue = new MessageQueue();
#if defined(DBG_SLLOOPER)
        LOGV("MessageQueue is created");
#endif // DBG_SLLOOPER
        if (mMessageQueue == NULL) {
            return false;
        }
    }

    return true;
}

void SLLooper::setName(const char* name)
{
    if (name == NULL) {
        return;
    }

    mLooperName->setTo((uint8_t*)name, static_cast<int32_t>(strlen(name)+1U));//20160718 hue.kim
}

char* SLLooper::getName()
{
    if (mLooperName->empty()) {
        return NULL;
    }

    return (char*)mLooperName->data();
}

error_t SLLooper::start(bool attachCurrentThread)
{
    if (mStarted) {
        LOGE("Looper is already started : %s", mLooperName->data());
        return E_ERROR;
    }

    if (attachCurrentThread) {
        if (mLooperThread != NULL) {
            LOGE("Looper already has thread");
            return E_ERROR;
        }
        android::Mutex::Autolock autoLock(mLock);

        if(mLooperName->size()) {
            set_process_name((const char*)mLooperName->data());
        }
        loop();

        return E_OK;
    }
    android::Mutex::Autolock autoLock(mLock);

    if (mLooperThread != NULL) {
        LOGE("Looper already has thread");
            return E_ERROR;
    }
    mLooperThread = new SLLooperThread(*this);
    android::status_t err = mLooperThread->run(
            mLooperName->empty() ? "SLLooper" : (char*)mLooperName->data(), 0);

    if (err != OK) {
        LOGE("SLLooper can not start due to some error");
        mLooperThread.clear();
    }

#ifdef Thread_Watcher_Handler
    /*
       Watchdog function
       */
#ifndef THandler_default_timer_enabled
    if (mReportSec != DEFAULT_REPORT_PERIOD_SEC)
#endif
    {
        LOGV("mReportSec %d watchdog for thread", mReportSec);
        sp<SLLooper> myself = this;
        mTHandler = new THandler(myself, mReportSec);
    }
#endif

#if defined(DBG_SLLOOPER)
    LOGV("SLLooper is started");
#endif // DBG_SLLOOPER

    return err;
}

error_t SLLooper::stop()
{
#ifdef Thread_Watcher_Handler
    mTHandler = NULL;
#endif
    if (mLooperThread == NULL) {
        return E_ERROR;
    }

    if (mLooperThread != NULL) {
        android::Mutex::Autolock autoLock(mLock);
        mStarted = false;
        messageQueue()->quit();
        mLooperThread->requestExit();
        mLooperThread = NULL;
    }

    return E_OK;
}

#ifdef Thread_Watcher_Handler
void SLLooper::setThreadWatcherTimeout(int sec)
{
    sp<SLLooper> myself = this;
    sp<SLLooper> curLooper = myLooper();
    LOGI("%s(sec = %d)  (myself == curr_Looper)? <%d>", __func__, sec, (myself == curLooper));
    LOGI("%s wll call dog_kick()", __func__);
    mTHandler->setTimeout(sec);
}
#endif

error_t SLLooper::join()
{
    if (mLooperThread == NULL) {
        LOGE("There is no thread for SLLooper");
        return E_ERROR;
    }
    mLooperThread->join();

    return E_OK;
}

void SLLooper::initTLSKey()
{
    int result = pthread_key_create(&gTLSKey, threadDestructor);
    LOG_ALWAYS_FATAL_IF(result != 0, "Could not allocate TLS key.");
}

void SLLooper::threadDestructor(void *st)
{
    SLLooper* const self = static_cast<SLLooper*>(st);

    if (self != NULL) {
        self->decStrong((void*)threadDestructor);
    }
}

void SLLooper::setForThread(const android::sp<SLLooper>& looper)
{
    android::sp<SLLooper> old = getForThread(); // also has side-effect of initializing TLS

    if (looper != NULL) {
        looper->incStrong((void*)threadDestructor);
    }
    pthread_setspecific(gTLSKey, looper.get());

    if (old != NULL) {
        old->decStrong((void*)threadDestructor);
    }
}

android::sp<SLLooper> SLLooper::getForThread() {
    int result = pthread_once(& gTLSOnce, initTLSKey);
    LOG_ALWAYS_FATAL_IF(static_cast<int8_t>(result) != 0, "pthread_once failed");

    return (SLLooper*)pthread_getspecific(gTLSKey);
}

android::sp<SLLooper> SLLooper::myLooper()
{
    android::sp<SLLooper> looper = SLLooper::getForThread();

    if (looper == NULL) {
        looper = new SLLooper();
        //if(mLegacyLooper == NULL || mMessageQueue == NULL)
        if (looper->prepare() == false) {
            LOGE("can not make looper");
            return NULL;
        }
    }

    return looper;
}

bool SLLooper::loop()
{
    android::sp<MessageQueue> queue = messageQueue();
    mStarted = true;

    while (mStarted) {
        android::sp<Message> message = queue->poll();

        if (message == NULL) {
            return false;
        }
        message->mHandler->dispatchMessage(message);
    }
    mStarted = false;

    return false;
}

void SLLooper::flush()
{
    //sp<Message> message = Message::obtain();
    //mMessageQueue->enqueueMessage(message, 0);
}

android::sp<MessageQueue> SLLooper::messageQueue()
{
    return mMessageQueue;
}

void SLLooper::dump()
{
    LOGI("============== dump Looper Infomation=======");
    LOGI("Looper name:%s, mStarted:%d", getName(), mStarted);
    LOGI("Queue address:0x%x", mMessageQueue.get());
    LOGI("===========================================");
}

} //namespace sl
