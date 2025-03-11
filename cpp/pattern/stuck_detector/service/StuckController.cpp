#include "Log.h"
#define LOG_TAG "StuckController"
#include "StuckDetectorManager.h"


StuckController::StuckController(StuckDetectorManager* manager) {
	manager_ = manager;
}

time_t StuckController::getCurrentTime(long& ms)
{
    timespec now;
    static time_t tv_sec = 0;
	//long ms;

    if (-1 == clock_gettime(CLOCK_MONOTONIC, &now)) {
        LOGE("Failed to call clock_gettime");
        return tv_sec;
    }
	ms = round(now.tv_nsec / 1.0e6);
    tv_sec = now.tv_sec;
    return tv_sec;
}

void StuckController::mainLooper() {
    std::cv_status waitStatus = std::cv_status::no_timeout;
    while (1) {
        //LOGI("[watchdog] main looper is running");

		std::unique_lock<std::recursive_mutex> lock(sendReceiveMutex_);

        std::chrono::steady_clock::time_point elapsed(std::chrono::steady_clock::now() + std::chrono::milliseconds(min_wait*1000));
		//LOGI("start waiting: min_wait=%d sec",min_wait);
		waitStatus = sendAndBlockCondition_.wait_until(lock, elapsed);
		//LOGI("exit waiting");
		if (getNotify == 1) {
			//LOGI("get notification, exit waiting");
            getNotify = 0;
		} else if (waitStatus == std::cv_status::timeout && (elapsed < std::chrono::steady_clock::now())) {
			//LOGI("timeout occur, exit waiting");
            next_wakeup_time = max_time;
            checkStuck();
            updateMinWait();
		}

    }
}


void StuckController::checkStuck() {
    //LOGI("check stuck");
    manager_->DBManager_->findStuck();
}
void StuckController::updateMinWait() {
    //LOGI("update min wait");
    sendReceiveMutex_.lock();

    long ms;

    //LOGI("updateMinWait 2: first_wakeup_time=%d, next_wakeup_time=%d",first_wakeup_time,next_wakeup_time);
    time_t first_wakeup_time = manager_->DBManager_->getMinLimitTime();

    if (first_wakeup_time < next_wakeup_time) {
        //int first_min_wait = static_cast<int>(first_wakeup_time - std::chrono::duration_cast<std::chrono::seconds>(now).count());
        int first_min_wait = static_cast<int>(first_wakeup_time - getCurrentTime(ms));

        next_wakeup_time = first_wakeup_time;
        min_wait = first_min_wait;
        //LOGI("min_wait=%d",min_wait);

        getNotify = 1;
	    sendAndBlockCondition_.notify_all();
        //LOGI("notify to main looper now");
        //sendReceiveMutex_.unlock();
    }
    sendReceiveMutex_.unlock();

}

