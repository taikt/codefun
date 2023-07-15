/**
* \file     LooperThread.h
* \brief    Declration of LooperThread class.
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

#ifndef SERVICE_LAYER_LOOPER_THREAD_H
#define SERVICE_LAYER_LOOPER_THREAD_H

#include <utils/Condition.h>
#include <utils/Thread.h>

#include "SLDefine.h"
#include "Typedef.h"
#include "utils/Handler.h"

/**
 * class LooperThread.
 *
 * This is to provide same functionality of Android LooperThread.
 * type T should be family of Handler class
 */

namespace sl {
template<class T /*extends Handler*/>
class LooperThread : public android::Thread
{
public:
    LooperThread() :
    mLooper(nullptr),
    mHandler(nullptr),
    mIsDone(false) 
    {}

    virtual ~LooperThread() {}

    android::sp<SLLooper> getLooper()
    {
        android::Mutex::Autolock _l(mLock);

        if (!mIsDone && mLooper == nullptr) {
            mCondition.wait(mLock);
        }

        return mLooper;
    }

    android::sp<T> getHandler() {
        android::Mutex::Autolock _l(mLock);

        if (!mIsDone && mHandler == nullptr) {
            mCondition.wait(mLock);
        }

        return mHandler;
    }

private:
    virtual bool threadLoop() {
        mLock.lock();
        mLooper = SLLooper::myLooper();
        mHandler = new T();
        mCondition.broadcast();
        mLock.unlock();
        mLooper->start(true);
        mLock.lock();
        mIsDone = true;
        //mHandler->removeCallbacksAndMessages();
        mLooper = nullptr;
        mHandler = nullptr;
        mLock.unlock();

        return true;
    }

private:
    android::sp<SLLooper> mLooper;
    android::sp<T> mHandler;
    android::Mutex mLock;
    android::Condition mCondition;
    bool mIsDone;

    DISALLOW_COPY_ASSIGN_CONSTRUCTORS(LooperThread)
};
} //namespace sl
#endif // SERVICE_LAYER_LOOPER_THREAD_H