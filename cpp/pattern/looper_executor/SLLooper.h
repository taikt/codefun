/**
* \file     SLLooper.h
* \brief    Declaration of SLLooper class.
*
* \details
*   This software is copyright protected and proprietary to
*   LG electronics. LGE grants to you only those rights as
*   set out in the license conditions. All other rights remain
*   with LG electronics.
* \author   sungwoo.oh
* \date     2015.06.08
* \attention Copyright (c) 2015 by LG electronics co, Ltd. All rights reserved.
*/


#ifndef SERVICE_LAYER_SLLOOPER_H
#define SERVICE_LAYER_SLLOOPER_H

#include <utils/Looper.h>
#include <utils/Mutex.h>

#include "SLDefine.h"
#include "Typedef.h"
#include "utils/Buffer.h"
#include "SLLooperThread.h"

#define Thread_Watcher_Handler
#define THandler_default_timer_enabled

namespace sl {
class MessageQueue;
#ifdef Thread_Watcher_Handler
class THandler;
#endif

/**
 * class SLLooper.
 */
class SLLooper : public android::RefBase
{
public:
    static const int   DEFAULT_REPORT_PERIOD_SEC = 60;
    SLLooper(
#ifdef Thread_Watcher_Handler
            const int reportPeriodSec = DEFAULT_REPORT_PERIOD_SEC
#endif
            );
    virtual ~SLLooper();

    bool prepare();
    static android::sp<SLLooper> myLooper();

    void setName(const char* name);
    char* getName();
    error_t start(bool attachCurrentThread);
    error_t stop();
    error_t join();

#ifdef Thread_Watcher_Handler
    void setThreadWatcherTimeout(int sec);
#endif

    android::sp<MessageQueue> messageQueue();

    void dump();

private:
    android::sp<MessageQueue> mMessageQueue;

    static void initTLSKey();
    static void threadDestructor(void *st);
    static void setForThread(const android::sp<SLLooper>& looper);
    static android::sp<SLLooper> getForThread();

    bool loop();
    void flush();

private:
    static const char* SERVICE_PREFIX;
    static const size_t SERVICE_PREFIX_SIZE;
    bool mStarted;

    android::Mutex mLock;
    android::sp<SLLooperThread> mLooperThread;
    android::sp<Buffer> mLooperName;

#ifdef Thread_Watcher_Handler
    int mReportSec;
    android::sp<THandler> mTHandler;
#endif

    DISALLOW_COPY_ASSIGN_CONSTRUCTORS(SLLooper)

    friend class SLLooperThread;
};
} // namespace sl
#endif // SERVICE_LAYER_SLLOOPER_H
