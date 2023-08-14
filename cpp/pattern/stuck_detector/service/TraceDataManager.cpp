#include "Log.h"

#define LOG_TAG "TraceDataManager"

#include "StuckDetectorManager.h"


TraceDataManager::TraceDataManager(StuckDetectorManager* manager, const std::shared_ptr<StuckController>& mController) {
	controller_ = mController;
	manager_ = manager;
}

void TraceDataManager::updateEntryinfo(std::shared_ptr<TraceInfo>& entry, int tid, int pgid,int timeoutsec, std::string func, std::string position,int clientSocket)
{
    //LOGI("[update_watchdog_info] pos=%s, funcName_=%s, tid=%d, timeout=%d",position.c_str(), func.c_str(),tid, timeoutsec);
    long ms;

    entry->timeout_ = timeoutsec;
    entry->lastkick_ = controller_->getCurrentTime(ms);
    entry->limit_ = entry->lastkick_ + entry->timeout_;
	entry->funcName_ = func;
    entry->position_ = position;
    entry->tid = tid;
    entry->pgid = pgid;
    entry->clientSocket = clientSocket;

    if (timeoutsec != -1) {
        entry->startTime_ = 1000 * entry->lastkick_ + ms;
    } else {
        entry->duration_ = (1000 * entry->lastkick_ + ms) - entry->startTime_;
    }
}

void TraceDataManager::addEntry(int tid, int pgid, int timeout, std::string key, std::string func, std::string pos,int clientSocket) {
    if (map.count(key)) {
        //LOGI("update existing entry");
        updateEntryinfo(map[key],tid,pgid,timeout,func,pos,clientSocket);
    } else {
        // add new element
        //LOGI("add new entry");
        std::shared_ptr<TraceInfo> watchdog_target = std::shared_ptr<TraceInfo>(new TraceInfo());
        map[key] = watchdog_target;
        updateEntryinfo(watchdog_target,tid,pgid,timeout,func,pos,clientSocket);
    }

    controller_->updateMinWait();
}

void TraceDataManager::printData()
{
    long ms;
    LOGI("CurrentTime= %zu sec>>", controller_->getCurrentTime(ms));

	LOGI("all trace infor:");
	for (auto it: map) {
        LOGI("tid=%d, pos=%s, funcName_=%s, timeout=%d",(it.second)->tid,(it.second)->position_.c_str(), (it.second)->funcName_.c_str(), (it.second)->timeout_);
        LOGI("lastkick=%zu, LimitTime= %zu",(it.second)->lastkick_, (it.second)->limit_);

        //LOGI("tid=%d, pos=%s, funcName_=%s, timeout=%d, lastkick=%zu, LimitTime= %zu",(it.second)->tid,(it.second)->position_.c_str(), (it.second)->funcName_.c_str(), (it.second)->timeout_,(it.second)->lastkick_, (it.second)->limit_);
        //LOGI("lastkick=%zu, LimitTime= %zu",(it.second)->lastkick_, (it.second)->limit_);
	}
	//LOGI("all trace infor end----");

}

time_t TraceDataManager::getMinLimitTime() {
    time_t first_wakeup_time = max_time;

	for (auto x: map) {
        if (first_wakeup_time > (x.second)->limit_ && (x.second)->timeout_ > 0) {
            first_wakeup_time = (x.second)->limit_;
        }
    }

    return first_wakeup_time;
}

void TraceDataManager::removeClient(const int& gpid) {
    //LOGI("remove client with gpid=%d",gpid);
    for (auto it=map.cbegin(); it!= map.cend();) {
        if (((*it).second)->pgid == gpid) {
            if (manager_->reactor_ != nullptr) {
                //LOGI("remove client with socket:%d",((*it).second)->clientSocket);
                manager_->reactor_->removeClient(((*it).second)->clientSocket);
            }
            it = map.erase(it);
        }
    }
}


void TraceDataManager::findStuck() {
    //printData();
    long ms;

    time_t curr_time = controller_->getCurrentTime(ms);

      //LOGI("[stuckDetector] current_time=%zu -------",curr_time);

      for (auto it=map.cbegin(); it!= map.cend();) {
          if (((*it).second)->limit_ <= curr_time && ((*it).second)->timeout_ > 0 ) {
               ((*it).second)->duration_ = (1000*curr_time + ms) - ((*it).second)->startTime_;
               LOGI("Stuck Detected!!  tid=%d, pos=%s,funcName_=%s,timeout=%d",((*it).second)->tid,((*it).second)->position_.c_str(),((*it).second)->funcName_.c_str(), ((*it).second)->timeout_);
               LOGI("lastkick=%zu, LimitTime= %zu, duration=%zu ms",((*it).second)->lastkick_,  ((*it).second)->limit_, ((*it).second)->duration_);
               //printData();
               removeClient(((*it).second)->pgid);
               kill(((*it).second)->pgid,SIGABRT);

               break;
               //abort();
               // TODO: kill remote process and remove all its entry
          } else if (((*it).second)->limit_ <= curr_time || ((*it).second)->timeout_ <= 0) {
               LOGI("released client,tid=%d,pos=%s,funcName_=%s,timeout_sec=%d",((*it).second)->tid,((*it).second)->position_.c_str(),((*it).second)->funcName_.c_str(), ((*it).second)->timeout_);
               LOGI("lastkick=%zu,Limit=%zu, duration=%zu ms",((*it).second)->lastkick_, ((*it).second)->limit_, ((*it).second)->duration_);

               it = map.erase(it);
          } else { // (x.second)->timeout_ > 0 or other cases
              ((*it).second)->duration_ = (1000*curr_time + ms) - ((*it).second)->startTime_;
              LOGI("on tracking, tid=%d, pos=%s, funcName_=%s, timeout=%d",((*it).second)->tid,((*it).second)->position_.c_str(), ((*it).second)->funcName_.c_str(), ((*it).second)->timeout_);
              LOGI("on tracking, lastkick=%zu, LimitTime= %zu, duration=%zu ms",((*it).second)->lastkick_,  ((*it).second)->limit_, ((*it).second)->duration_);
              ++it;
          }
      }
}


