/**
* \file     SLLooperThread.cpp
* \brief    Implementation of SLLooperThread class.
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


#include "Log.h"
#include "utils/SLLooper.h"
#include "utils/SLLooperThread.h"

#undef LOG_TAG
#define LOG_TAG "SLLooperThread"

namespace sl {
android::status_t SLLooperThread::readyToRun()
{
    mLooper.prepare();
    return android::Thread::readyToRun();
}

bool SLLooperThread::threadLoop()
{
    return mLooper.loop();
}
} //namespace sl