/**
* \file     SLLooperThread.h
* \brief    Declaration of SLLooperThread class.
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


#ifndef SERVICE_LAYER_SLLOOPER_THREAD_H
#define SERVICE_LAYER_SLLOOPER_THREAD_H

#include <utils/Thread.h>

#include "SLDefine.h"
#include "Typedef.h"

using android::sp;
using android::status_t;
using android::Thread;

namespace sl {
class SLLooper;

/**
 * class SLLooperThread.
 *
 *
 */
class SLLooperThread : public android::Thread
{
public:
    SLLooperThread(SLLooper& looper) : mLooper(looper) {}
    virtual android::status_t readyToRun();

protected:
    virtual ~SLLooperThread() {}

private:
    virtual bool threadLoop();

private:
    SLLooper& mLooper;

    DISALLOW_COPY_ASSIGN_CONSTRUCTORS(SLLooperThread);
};
} //namespace sl
#endif // SERVICE_LAYER_SLLOOPER_THREAD_H