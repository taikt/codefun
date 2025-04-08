#pragma once

#include <boost/asio.hpp>
#include <mutex>
#include <thread>
#include <deque>
#include <unordered_map>
#include <condition_variable>
#include <functional>
#include <string>
#include <unordered_map>
#include <sys/syscall.h>


#include <utils/Timer.h>
#include <utils/Handler.h>
#include <utils/SLLooper.h>
#include <utils/Vector.h>
#include <utils/Mutex.h>
#include <time.h>
#include <math.h>

#include <signal.h>
#include <unistd.h>


#include "JobQueue.hpp"
#include "WorkerThread.hpp"
#include "Dispatcher.hpp"
#include "Reactor.h"

//const time_t max_time = (time_t)((unsigned time_t)(~(time_t)0) >> 1);
const time_t max_time = 0x7FFFFFFF;
#define MIN_WAIT 120
#define THREAD_COUNT 2


using namespace std;

class TraceInfo;
class StuckDetectorManager;
class StuckController;

class IStuckDetector {
	public:
		virtual void heartBeat(int timeout,std::string file, int line, std::string func, int tid, int pgid, int clientSocket)=0;
};

class TraceInfo {
    public:
        TraceInfo(){funcName_ = ""; position_="";}

        int timeout_; //in second
        int limit_;
        int lastkick_;
		std::string funcName_;
		std::string position_;
		long startTime_;
		long duration_; //in milisecond
		int tid;
		int pgid;
		int clientSocket;
};


class TraceDataManager {
	public:
		TraceDataManager(StuckDetectorManager* manager, const std::shared_ptr<StuckController>& mController);
		void updateEntryinfo(std::shared_ptr<TraceInfo>& entry, int tid, int pgid,int timeoutsec, std::string func, std::string position,int clientSocket);
		void addEntry(int tid,int pgid,int timeout, std::string key, std::string func, std::string pos,int clientSocket);
		void printData();
		void findStuck();
		time_t getMinLimitTime();
		void removeClient(const int& gpid);


	private:
		typedef std::unordered_map<std::string, std::shared_ptr<TraceInfo>> watchdog_info_map;
		watchdog_info_map map;
		std::shared_ptr<StuckController> controller_;
		StuckDetectorManager* manager_;

};


class StuckController {
	public:
		StuckController(StuckDetectorManager* manager);
		void checkStuck();
		void mainLooper();
		void updateMinWait();
		time_t getCurrentTime(long& ms);

	private:
		std::recursive_mutex sendReceiveMutex_;
		std::condition_variable_any sendAndBlockCondition_;
		int getNotify = 0;
		time_t next_wakeup_time = max_time;
		int min_wait=MIN_WAIT;
		StuckDetectorManager* manager_;
};


class StuckDetectorManager: public IStuckDetector {
public:
	StuckDetectorManager();
	static StuckDetectorManager* getInstance();
	void heartBeat(int timeout,std::string file, int line, std::string func, int tid, int pgid, int clientSocket) override;
	void onInit();
	void setReactor(Reactor * reactor);


private:
	static StuckDetectorManager* singleton_;
	std::shared_ptr<Dispatcher> taskDispatcher_;
	std::shared_ptr<TraceDataManager> DBManager_;
	std::shared_ptr<StuckController> controller_;
	int start=0;
	static std::mutex instanceMutex;
	Reactor * reactor_;

public:
	friend StuckController;
	friend TraceDataManager;

};





