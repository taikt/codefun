#include "Log.h"
#define LOG_TAG "StuckDetectorManager"

#include "StuckDetectorManager.h"



StuckDetectorManager* StuckDetectorManager::singleton_= nullptr;
std::mutex StuckDetectorManager::instanceMutex;


StuckDetectorManager::StuckDetectorManager() {
    //const int threadSize_ = 5;
    taskDispatcher_ = std::shared_ptr<Dispatcher>(new Dispatcher(THREAD_COUNT));
    controller_ = std::shared_ptr<StuckController>(new StuckController(this));
    DBManager_ = std::shared_ptr<TraceDataManager>(new TraceDataManager(this,controller_));
	//onInit();
}

void StuckDetectorManager::setReactor(Reactor * reactor) {
	reactor_ = reactor;
}

void StuckDetectorManager::onInit() {
    taskDispatcher_->deliverTask([=]{StuckDetectorManager::getInstance()->controller_->mainLooper();});
}

StuckDetectorManager* StuckDetectorManager::getInstance() {
    std::lock_guard<std::mutex> guard(StuckDetectorManager::instanceMutex);
    if(singleton_ == nullptr) {
        singleton_ = new StuckDetectorManager();
        //this->onInit();
    }

    return singleton_;
}

void StuckDetectorManager::heartBeat(int timeout, std::string file, int line, std::string func, int tid, int pgid,int clientSocket) {
    //LOGI("addTracer: func=%s, timeout=%d",func.c_str(),timeout);
	/*
    if (start == 0) {
        taskDispatcher_->deliverTask([=]{StuckDetectorManager::getInstance()->controller_->mainLooper();});
        start = 1;
    }
    */

    std::string position,key;
    //int current_tid = syscall(SYS_gettid);

    std::size_t found = file.find_last_of("/\\");
    if (found != std::string::npos) {
        position = file.substr(found+1);
    }

    position += ":";
    position += to_string(line);

    key = position;
    key += "_" + to_string(tid);


    taskDispatcher_->deliverTask([=]{StuckDetectorManager::getInstance()->DBManager_->addEntry(tid,pgid,timeout,key,func,position,clientSocket);});
}




