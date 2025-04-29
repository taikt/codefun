/**
* \file    leopard.cpp
* \brief     This is process created firstly on the Service Layer
*
* \details
*    This software is copyright protected and proprietary to
*    LG electronics. LGE grants to you only those rights as
*    set out in the license conditions. All other rights remain
*    with LG electronics.
* \author       sungwoo.oh
* \date       2015.03.18
* \attention Copyright (c) 2015 by LG electronics co, Ltd. All rights reserved.
*/


#include <cstddef>
#include <execinfo.h>
#include <fcntl.h>
#include <fstream>
#include <string>
#include <system_property.h>
#include <unistd.h>

#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>

#include <binder/IServiceManager.h>
#include <cutils/process_name.h>
#include <cutils/sockets.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>

#if defined(HAVE_PRCTL)
#include <sys/prctl.h>
#endif

#include <grp.h>
/* glibc doesn't implement or export either gettid or tgkill. */
#include <unistd.h>
#include <sys/syscall.h>
#include <systemd/sd-daemon.h>

#include "Log.h"
#include "ApplicationTable.h"
#include "corebase/application/ApplicationFactory.h"
#include "corebase/application/Application.h"
#include "corebase/Version.h"
#include "corebase/BootMode.h"
#include "services/ApplicationManagerService/IApplicationManagerService.h"
#include "services/ApplicationManagerService/IApplicationManagerServiceType.h"
#include "daemon/start/startd/leopard.h"

#define APP_PROC_PATH "/usr/bin/app_proc"
#define ADDSERVICE_NAME_MAX 125U
#define ADDDEAMON_NAME_MAX 125U

#if defined(PLATFOTM_RESET_BY_POWMGR)
#include <utils/RefBase.h>
#include <binder/IServiceManager.h>
#include <binder/IBinder.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <utils/Mutex.h>
#include <services/PowerManagerService/IPowerManagerService.h>
#endif


#if defined(PLATFORM_RESET_BY_BSP)
#include "utils/PosixCmd.h"
#endif

#undef LOG_TAG
#define LOG_TAG "Leopard"

static LeopardServiceInfo ServiceProcessInfo[] = {
    #include "daemon/start/startd/services.h"
};

/*
static LeopardDaemonInfo daemon_process_info[] = {
    #include "daemon/start/startd/sldaemons.h"
};
*/
extern char **environ;

Leopard::Leopard()
    :server_socket(-1), boot_mode(0), launch_service_n(0), launch_service_n_lev2(0),
    mWatcdogTimer(NULL), mTimerImp(NULL)
{
}

Leopard::~Leopard(){

}

static Leopard* leopard;

/**
*  start Leopard process.
*  This will be called from entry point of Service layer.
*
* \param        NONE
* \return        NONE
* \note        none.
*/
void Leopard::launch()
{
    SYSLOGI("Leopard::launch()");
    set_process_name("leopard");

    leopard = new Leopard();
    leopard->initLeopard();
    leopard->mainLoop();
}

/**
*  initLeopard.
*
* \param        NONE
* \return        NONE
* \note        Init Leopard process during boot up.
*/
void Leopard::initLeopard()
{
    buildVersion();
	leopard->initNotBootTimer();

    /* init socket to fork process */
    registerServerSocket();

    /* fork servicelayer's Daemons */
    //forkDaemonByExec();

    /* fork servicelayer's Servcies */
    forkServiceLayerByExec(true, SVC_ID_MAX);

    sd_notify(0, "READY=1");
}

/**
*  mainLoop.
*  Leopard will wait some of request from ServiceLayer at here.
*  If ServiceLayer needs to make application,
*  It will send the request via socket and Leopard will fork that.
*
*  When Leopard get some request from ServiceLayer,
*     - make ForkSession and add it into list
*     - perform operation of ForkSession if ForkSession exists in the list.
*     - wait again once done.
*
* \param[in]    isApplication TRUE, if this is for application.
* \return         pid_t            process id forked
* \note    none.
*/
void Leopard::mainLoop()
{
    fd_set fds;
    int32_t fd[10];
    int32_t fdsLength;
    int32_t nfds;
    int32_t count;

    if (server_socket < 0) {
        SYSLOGE("Unavailable status!!!!");
        return;
    }

    FD_ZERO(&fds);

    /* init fd */
    fd[0] = server_socket;
    fdsLength = 1;

#ifdef DEBUG_LEOPARD
    SYSLOGE("start loop:%d\n", server_socket);
#endif

    while(true) {
        nfds = 0;

        for (int32_t i = 0; i < fdsLength; i++) {
            FD_SET(fd[i], &fds);
            if (fd[i] >= nfds) {
                nfds = fd[i]+1;
            }
        }

        count = select(nfds, &fds, 0, 0, 0);
        if (count < 0) {
            //error happens. How to recover this?
            SYSLOGE("select() count = %d, errno = %d\n", count, errno);
            continue;
        }

        if(count >= 0) {
            int32_t i = 0;

#ifdef DEBUG_LEOPARD
            SYSLOGE("c:%d\n", count);
#endif

            for (i = 0; i < fdsLength; i++) {
                if (FD_ISSET(fd[i], &fds)) {
                    break;
                }
            }

#ifdef DEBUG_LEOPARD
               SYSLOGE("i:%d\n", i);
#endif

            if(i == 0) {
                /* This is the connection for Server socket
                            So, make client fd to read next request */
                struct sockaddr addr;
                socklen_t len = 0U;
                int32_t client_fd = 0;

                len = sizeof(addr);
                client_fd = accept(server_socket, &addr, &len);

                if (client_fd < 0) {
                    //error happens. How to recover this?
                    continue;
                }
                fcntl(client_fd, F_SETFD, FD_CLOEXEC);

#ifdef DEBUG_LEOPARD
                SYSLOGE("create session:%d\n", client_fd);
#endif

                performRequest(client_fd);
                close(client_fd);
            }
        }
    }
}

/**
*  start service which has pid of parameter
*
* \param        NONE
* \return        NONE
* \note        none.
*/
void Leopard::re_start_service(pid_t pid)
{
    if (leopard == NULL) {
        SYSLOG_FATAL("start_service but, No leopard instance");
    }

    leopard->_re_start_service(pid);
}

void Leopard::_re_start_service(pid_t pid) /* should we  separate  this funcion in case servce and app? */
{
    LeopardServiceList* svc = service_find_by_pid(pid);

    if (svc == nullptr) {
        LOGE("can not start service(pid:%d)", pid);
        return;
    }

    if (checkSystemWatchdogBursted(*svc) == true) {
      return ;
    }
    LOGE("<<<< Tiger RESTART Process >>>> name:%s id:%d", svc->name.c_str(), svc->service_id);

    if (svc->service_id >= SERVICE_BASE && svc->service_id <= SVC_ID_MAX ) { // in case services
        forkServiceLayerByExec(false, svc->service_id);
    } else { // in case application restart
        forkApplicationByExec(svc->name, false, svc->lable); // in case with launch lable
    }

#if defined(DEBUG_LEOPARD)
    dumpService();
#endif ///DEBUG_LEOPARD
}

static const int32_t FTM_CMDLINE_SIZE = 1024;
static const char KERNEL_CMDLINE_PATH[] = "/proc/cmdline";
static const char TIGER_BOOTMODE_PATH[] = "/tmp/bootmode_cmd";
static const char normal_mode[] = "normal";
static const char factory_mode[] = "factory";
static const char download_mode[] = "download";
static const char developer_mode[] = "developer";
static const char nadtest_mode[] = "nad_test";
static const char factory_reset_mode[] = "factory_reset";
struct bootmode_list {
    char* name;
    int32_t   type;
};

static struct bootmode_list mode_list[] = {
    {normal_mode,    BOOT_MODE_NORMAL  },
    {factory_mode,   BOOT_MODE_FACTORY },
    {download_mode,  BOOT_MODE_DOWNLOAD },
    {developer_mode, BOOT_MODE_DEVELOPER },
    {nadtest_mode,   BOOT_MODE_NADTEST },
    {factory_reset_mode,   BOOT_MODE_FACTORY_RESET },
    0
};

static int32_t get_cmdline_value(const char* str_file, const char* cmd_name, char* value, int32_t val_len)
{
    int32_t fd = -1;
    int32_t result = -1;
    char buf[FTM_CMDLINE_SIZE + 1] = {0, };
    char *cutstr = NULL;

    fd = open(str_file, O_RDONLY);
    if (fd < 0) {
        SYSLOGE("%s(): file[%s] open failed\n", __func__, str_file);
        return -1;
    }

    result = read(fd, buf, FTM_CMDLINE_SIZE);
    close(fd);
    if (result < 0) {
        SYSLOGE("%s: file read failed\n", __func__);
        return -1;
    }

    cutstr = strstr(buf, cmd_name);
    if (cutstr == NULL) {
        SYSLOGE("%s(): %s doesn't exist in [%s]\n", __func__, cmd_name, str_file);
        return -1;
    } else {
        char* p = cutstr + strlen(cmd_name);
        char* q = strpbrk(p, " \t\n\r");
        if (q != NULL) {
            *q = 0;
        }
        SYSLOGI("%s(): find [%s] in [%s]\n", __func__, cmd_name, str_file);
        memcpy(value, p, val_len);
    }
    return 0;
}

int32_t count_services_should_launched(int32_t mode)
{
    uint32_t i;
    int32_t cbit;
    int32_t cnt = 0;
    uint32_t size = sizeof(ServiceProcessInfo) / sizeof(LeopardServiceInfo) - 1;

    switch(mode) {
        case BOOT_MODE_FACTORY:
            cbit = LAUNCH_FACTORY;
            break;
        case BOOT_MODE_DOWNLOAD:
            cbit = LAUNCH_DOWNLOAD;
            break;
        case BOOT_MODE_DEVELOPER:
            cbit = LAUNCH_DEVELOPER;
            break;
        case BOOT_MODE_NADTEST:
            cbit = LAUNCH_NADTEST;
            break;
        case BOOT_MODE_FACTORY_RESET:
            cbit = LAUNCH_FACTORY_RESET;
            break;
        case BOOT_MODE_NORMAL:
        default:
            cbit = LAUNCH_NORMAL;
            break;
    }

    for (i = 0; i < size; i++) {
        if (cbit & ServiceProcessInfo[i].service_id)
            cnt++;
    }
    return cnt;
}


int32_t count_services_should_launched_lev1 (int32_t mode)
{
    uint32_t i;
    int32_t cbit;
    int32_t cnt = 0;
    uint32_t size = (uint32_t)(sizeof(ServiceProcessInfo) / sizeof(LeopardServiceInfo) - 1);

    switch(mode) {
        case BOOT_MODE_FACTORY:
            cbit = LAUNCH_FACTORY;
            break;
        case BOOT_MODE_DOWNLOAD:
            cbit = LAUNCH_DOWNLOAD;
            break;
        case BOOT_MODE_DEVELOPER:
            cbit = LAUNCH_DEVELOPER;
            break;
        case BOOT_MODE_NADTEST:
            cbit = LAUNCH_NADTEST;
            break;
        case BOOT_MODE_FACTORY_RESET:
            cbit = LAUNCH_FACTORY_RESET;
            break;
        case BOOT_MODE_NORMAL:
        default:
            cbit = LAUNCH_NORMAL;
            break;
    }

    for (i = 0; i < size; i++) {
        if (( cbit & (int32_t)ServiceProcessInfo[i].service_id ) != 0 )
       	{
       		if ( ServiceProcessInfo[i].activeNumber == Level_1 )
            cnt++;
        }
    }
    return cnt;
}

int32_t count_services_should_launched_lev2 (int32_t mode)
{
    uint32_t i;
    int32_t cbit;
    int32_t cnt = 0;
    uint32_t size = sizeof(ServiceProcessInfo) / sizeof(LeopardServiceInfo) - 1;

    switch(mode) {
        case BOOT_MODE_FACTORY:
            cbit = LAUNCH_FACTORY;
            break;
        case BOOT_MODE_DOWNLOAD:
            cbit = LAUNCH_DOWNLOAD;
            break;
        case BOOT_MODE_DEVELOPER:
            cbit = LAUNCH_DEVELOPER;
            break;
        case BOOT_MODE_NADTEST:
            cbit = LAUNCH_NADTEST;
            break;
        case BOOT_MODE_FACTORY_RESET:
            cbit = LAUNCH_FACTORY_RESET;
            break;
        case BOOT_MODE_NORMAL:
        default:
            cbit = LAUNCH_NORMAL;
            break;
    }

    for (i = 0; i < size; i++) {
        if (cbit & ServiceProcessInfo[i].service_id)
       	{
       		if ( ServiceProcessInfo[i].activeNumber == Level_2 )
            cnt++;
        }
    }
    return cnt;
}

void Leopard::buildVersion()
{
    std::ifstream is;
    std::string version;
    int32_t prop_ret = E_OK;

    is.open(BUILD_VERSION_PATH, std::ios::in);

    if (!is.is_open()) {
        SYSLOGI("** Can not open [%s] file **", BUILD_VERSION_PATH);
    } else {
        std::getline(is, version);
        SYSLOGI("** build version : [%s] **", version.c_str());

        prop_ret = set_open_property(PROPERTY_BUILD_VERSION, version.c_str(), MEMORY);
        is.close();
    }

    char mode[20] = {0, };
    if (get_cmdline_value(KERNEL_CMDLINE_PATH, BOOTMODE_CMD, mode, 20) < 0) {
        if (get_cmdline_value(TIGER_BOOTMODE_PATH, BOOTMODE_CMD, mode, 20) < 0) {
            sprintf(mode, "normal");
            SYSLOGI("lge.bootmode string is not found -> set normal");
        }
    }
    int32_t cmd_s_len = strnlen(mode, sizeof(mode));
    int32_t i;
    for (i = 0; mode_list[i].name; i++) {
        int32_t mode_s_len = strnlen(mode_list[i].name, sizeof(mode));
        if (memcmp(mode, mode_list[i].name, mode_s_len) == 0 && cmd_s_len == mode_s_len) {
            boot_mode = mode_list[i].type;
            break;
        }
    }
    if (boot_mode == BOOT_MODE_NOT_DEFINED) {
        SYSLOGE("Unknown bootmode = %s --> normal", mode);
        sprintf(mode, "normal");
        boot_mode = BOOT_MODE_NORMAL;
    }
    prop_ret = set_open_property(PROPERTY_BOOTMODE, mode, MEMORY);
//     launch_service_n = count_services_should_launched(boot_mode);

    launch_service_n = count_services_should_launched_lev1(boot_mode);
    launch_service_n_lev2= count_services_should_launched_lev2(boot_mode);

    SYSLOGI("%s: bootmode(0x%x) string[%s] ret set_prop=%d, launch_service_n=%d\n",
            __func__, boot_mode, mode, prop_ret, launch_service_n);
}

#define SMACK_LABEL_LEN 255

static int32_t smack_check(){//check kernel whether smack is supported
    int32_t ret = -1;
    char* file_name = "/proc/filesystems";//list of supported filesystems
    char buffer[100];

    FILE * f = fopen(file_name, "r");
    if(f != NULL){
        while(fgets(buffer, sizeof(buffer), f) != NULL){
            if(strstr(buffer, "smack") != NULL){
                ret = 0;
                break;
            }
        }
        fclose(f);
    }
    return ret;
}

static int32_t label_sanity_check(const char* label){
    int32_t ret = -1;

    if((label!=NULL)){
        int32_t len = strlen(label);
        if(len <= SMACK_LABEL_LEN){
            ret = 0;
        }
    }
    return ret;
}

static int32_t set_label(const char* label){
    int32_t ret = -1;
    char* actual_label = "_";//run as floor by default

    if(label_sanity_check(label) == 0){
        actual_label = label;
    }

    FILE * f = fopen("/proc/self/attr/current", "w");
    if(f != NULL){
        fprintf(f, actual_label);
        fclose(f);
        ret = 0;
    }
    return ret;
}

std::string &ltrim(std::string &s);
std::string &rtrim(std::string &s);
std::string &trim(std::string &s);

// trim from start
std::string &ltrim(std::string &s) {
    s.erase(s.begin(), find_if(s.begin(), s.end(), not1(std::ptr_fun<int32_t, int32_t>(isspace))));
    return s;
}

// trim from end
std::string &rtrim(std::string &s) {
    s.erase(find_if(s.rbegin(), s.rend(), not1(std::ptr_fun<int32_t, int32_t>(isspace))).base(), s.end());
    return s;
}

// trim from both ends
std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}

int32_t Leopard::forkServiceLayerByExecName(const LeopardServiceInfo& target_service)
{
    LOGV("forkServiceLayerByExecName %s", target_service.service_name);
    pid_t pid = fork();

    if(pid == 0) {
        if((smack_check() == 0) && (set_label("_") != 0)){
            /*Process can't stay running privileged so quit here*/
            _exit(0);
        }

        int32_t args_size = 5U;
        char **args = new char*[args_size]; // fixed misra c++ 5-2-8

        for (int32_t i = 0; i < args_size; i++) {
            args[i] = new char[ADDSERVICE_NAME_MAX];
            memset(args[i], 0, sizeof(char)*ADDSERVICE_NAME_MAX);
        }

        strncpy(args[0], target_service.service_name, ADDSERVICE_NAME_MAX-1); // fixed misra c++ 5-2-5
        strncpy(args[1], target_service.arg[0], ADDSERVICE_NAME_MAX-1);       // fixed misra c++ 5-2-5
        strncpy(args[2], target_service.arg[1], ADDSERVICE_NAME_MAX-1);       // fixed misra c++ 5-2-5
        strncpy(args[3], target_service.arg[2], ADDSERVICE_NAME_MAX-1);       // fixed misra c++ 5-2-5
        args[4] = NULL;

        for(int32_t i = 0; environ[i] != NULL; i++){
            ALOGI("Lunch environ %d %s",i,environ[i]);
        }

        if(execve(args[0], args, environ)) {
            ALOGE("execve forkServiceLayer failed (%s)", strerror(errno));
            ALOGE("args[%s], args[%s], args[%s], args[%s], args[%s]", args[0], args[1], args[2], args[3], args[4]);
        }

        ALOGE("Should never get here as creating ServiceLayer");

        for (int32_t i = 0; i < args_size; i++) {
            delete [] args[i];
        }
        delete [] args;

        _exit(0);
    }

    return pid;
}

int32_t Leopard::getcurrentruntime(void)
{
    timespec now;

    if (-1 == clock_gettime(CLOCK_MONOTONIC, &now)) {
        LOGE("Failed to call clock_gettime");
        return E_ERROR;
    }

    return (int32_t)now.tv_sec;
}

void Leopard::initTimer()
{
    mTimerImp = new TimeoutHandlerImp (*this);
    mWatcdogTimer = new Timer(mTimerImp, 0x00AA);
    mWatcdogTimer->setDuration(1U,3U);
    mWatcdogTimer->start();
}

void Leopard::TimeoutHandlerImp::handlerFunction (int32_t timer_id)
{
    switch (timer_id) {
        case 0x00AA:
            mService.checkwatchdog();
        break;

        default:
        break;
    }
}

void Leopard::BootTimeoutHandlerImp::handlerFunction (int32_t timer_id)
{
    switch (timer_id) {
        case TIMER_ID_FORCE_BOOTCOMPLETEPRE:
            mService.forceBootCompletePre();
        break;

        case TIMER_ID_REMIND_BOOTCOMPLETEPRE:
            mService.logBootCompletePre();
        break;

        case TIMER_ID_NOT_BOOTCOMPLETEPRELIST:
            mService.notBootCompleteforRightTime();
        break;

        default:
        break;
    }
}

bool Leopard::checkSystemWatchdogBursted(LeopardServiceList& svc)
{
    svc.failoccurtime[svc.failarryindex] = getcurrentruntime();
    LOGE("name[%s], restart_need_reset(%d)", svc.name.c_str(), svc.restart_need_reset);

    if (is_limitBurst(svc) || svc.restart_need_reset == true) {
        LOGE("[%s] PID(%d) Process Busted fail count. Need Restart Tiger Platform.", svc.name.c_str(), svc.pid);
        platformReset();
        return true;
    }
    svc.failarryindex++;

    if (svc.failarryindex >= FAILARRYCOUNT) {
        svc.failarryindex = 0;
    }

    return false;
}

bool Leopard::platformReset()
{
#if defined(PLATFOTM_RESET_BY_PLATFORM)
    LOGE("platformReset Reset Start!!");
    set_ready_complete(0);
	set_ready_complete_lev2(0);
    clearService(0);
    forkServiceLayerByExec(false, SVC_ID_MAX);
#elif defined(PLATFOTM_RESET_BY_POWMGR)
    android::sp<IPowerManagerService> powerManager = android::interface_cast<IPowerManagerService>
        (android::defaultServiceManager()->getService(android::String16(POWER_SRV_NAME)));
   if (powerManager != NULL) {
        powerManager->reboot(REBOOT_BY_USER_PROC);
    }else {
        LOGE("platformReset Fadiled");
    }
#elif defined(PLATFORM_RESET_BY_BSP)
    run_cmd("reboot");
#else
    LOGE("platformReset Disabled");
#endif
    return true;
}

bool Leopard::resetAllApplications()
{
    std::lock_guard<std::mutex> lock(mLock);
    if (service_list.empty()) {
        LOGE("resetAllApplications() service list is empty!!");
        return false;
    }

    std::list<LeopardServiceList*>::iterator service_it;
    for (service_it = service_list.begin(); service_it != service_list.end(); ) {
        if ((*service_it)->service_id == APPID_BASE) {
            killApplicationByPid((*service_it)->name, (*service_it)->pid);

            delete (*service_it);
            service_it = service_list.erase(service_it);
        } else {
            service_it++;
        }
    }
    sp<IApplicationManagerService> app = interface_cast<IApplicationManagerService>
        (defaultServiceManager()->getService(String16(APPLICATION_SRV_NAME)));
    app->enumerateAvailableApplication(true);

    return true;
}

bool Leopard::checkwatchdog()
{
    std::lock_guard<std::mutex> lock(mLock);
    if (service_list.empty()) {
        LOGE("service list is empty!!");
        return false;
    }
    int32_t gapTime = 0;
    std::list<LeopardServiceList*>::iterator service_it;
    for (service_it = service_list.begin(); service_it != service_list.end(); ++service_it) {
        if (DISABLE_WATCHDOG_TIME != (*service_it)->watchdogtime) {
            gapTime = getcurrentruntime() - (*service_it)->lastkicktime;

            if (gapTime > (*service_it)->watchdogtime) {
                LOGE("Watchdog OCCUR!! %s [PID:%d], %d, %d", (*service_it)->name.c_str(), (*service_it)->pid, gapTime, (*service_it)->watchdogtime);
                (*service_it)->lastkicktime = getcurrentruntime();
                if((*service_it)->service_id == APPID_BASE) {
                    kill((*service_it)->pid, SIGABRT);
                }
            }
        }
    }

    return true;
}

bool Leopard::kickwatchdog(pid_t pid)
{
    LeopardServiceList* svc = service_find_by_pid(pid);

    if (svc == nullptr) {
        LOGE("kickwatchdog can not start service(pid:%d)", pid);
        return false;
    } else {
        svc->lastkicktime = getcurrentruntime();
    }

    return true;
}

bool Leopard::is_limitBurst(LeopardServiceList& svc)
{
    int16_t check_lasttime=0;
    int32_t diff_time=0;

    if (svc.failarryindex > svc.StartLimitBurst) {
        check_lasttime = svc.failarryindex - svc.StartLimitBurst;
    } else {
        check_lasttime = FAILARRYCOUNT - (svc.StartLimitBurst - svc.failarryindex);

        if (check_lasttime == FAILARRYCOUNT) {
            check_lasttime = 0x00;
        }
    }

    LOGE("check LimitBust %d - %d = %d  StartLimitInterval=%d ", svc.failoccurtime[svc.failarryindex],
        svc.failoccurtime[check_lasttime],
        svc.failoccurtime[svc.failarryindex] - svc.failoccurtime[check_lasttime],
        svc.StartLimitInterval);

    diff_time = svc.failoccurtime[svc.failarryindex] - svc.failoccurtime[check_lasttime];

    if (diff_time > svc.StartLimitInterval) {
        return false;
    } else {
        if (svc.failoccurtime[check_lasttime] == 0x00) {
            return false;
        } else {
            return true;
        }
    }
}

/**
*  forkApplicationByExec.
*
* \param[in] app_id     application id to fork
* \return        NONE
* \note    This will be used only to fork application process.
*/
void Leopard::forkApplicationByExec(const std::string& app, bool add_list,const std::string& launchLable)
{
    int32_t args_size = 5;
    if (app.empty()) {
        SYSLOGI("invalid app name[%s]", app.c_str());
        return;
    }

    SYSLOGI("forkApplicationByExec app[%s], add_list(%d)", app.c_str(), add_list);
    registerSigHandler();

    pid_t pid;
    pid = fork();

    if (pid == 0) {
        std::string label = "_";
        if (strnlen(launchLable.c_str(),100) > 0x00 ) {
            //label comes in format "-l label". we need to cut off -l and trim
            size_t pos = launchLable.find("-l");
            if(pos != std::string::npos){
                label = launchLable.substr(pos+2U);
                trim(label);
           }
        }

        if((smack_check() == 0) && (set_label(label.c_str()) != 0)){
            /*Process can't stay running privileged so quit here*/
            _exit(0);
        }

        /* application process */
        SYSLOGI("child app [%s]\n", app.c_str());
        releaseSigHandler();

        int32_t args_length = 128;
        char **args = new char*[args_size]; // fixed misra c++ 5-2-8

        for (int32_t i = 0; i < args_size; i++) {
            args[i] = new char[args_length];
            memset(args[i], 0, sizeof(char)*args_length);
        }

        strncpy(args[0], APP_PROC_PATH, args_length-1); // fixed misra c++ 5-2-5
        strncpy(args[1], "-i", args_length-1);          // fixed misra c++ 5-2-5
        strncpy(args[2], app.c_str(), args_length-1);   // fixed misra c++ 5-2-5
        strncpy(args[3], "-c", args_length-1);          // fixed misra c++ 5-2-5
        args[4] = NULL;

        for(int32_t i = 0; environ[i] != NULL; i++){
            ALOGI("Lunch environ %d %s",i,environ[i]);
        }

        if (execve(args[0], args, environ)) {
            ALOGE("execve forkApp failed (%s)", strerror(errno));
        }

        ALOGE("Should never get here as creating Apps");

        for (int32_t i = 0; i < args_size; i++) {
            delete [] args[i];
        }
        delete [] args;

        _exit(0);
    }

    if (add_list) {
        /* service id =APPID_BASE mean It's Application. */
        addService(app, APPID_BASE, pid, APP_LIMIT_INTERVAL, APP_LIMIT_BURST, APP_WATCHDOG_TIME, false, false, false, 0, 0,launchLable);
    } else {
        LeopardServiceList* svc = service_find_by_name(app);

        if (svc != nullptr) {
            svc->pid = pid;
            svc->watchdogtime = APP_WATCHDOG_TIME;
            svc->lastkicktime = getcurrentruntime();
        }
    }
    LOGD("added applications list size(%d)", service_list.size());
}

/**
*  Return service Process name
*
* \param        NONE
* \return        NONE
* \note    This will be return service name that should be forked process.
*/
const char* Leopard::get_process_servicename(int32_t count)
{
    return ServiceProcessInfo[count].service_name;
}

/**
*  forkServiceLayer.
*
* \param        add list : true : save service array svc_id : SVC_ID_MAX :  all services
* \return        NONE
* \note    This will be used only to fork ServiceLayer process.
*/
void Leopard::forkServiceLayerByExec(bool add_list, int32_t svc_id)
{
    ALOGE("start to fork servicelayer add_list:%d  svc_id:%d", add_list, svc_id);
    int32_t fork_service_count = 0;
    int32_t local_watchdogtime = SVC_WATCHDOG_TIME;

    registerSigHandler();
    ALOGE("local_watchdogtime =%d", SVC_WATCHDOG_TIME);

	std::string launchLable = "";

    while (get_process_servicename(fork_service_count) != NULL) {
        LeopardServiceInfo *local_info = &(ServiceProcessInfo[fork_service_count]);

        int32_t service_id = (~LAUNCH_BIT_ALL & local_info->service_id);
        int32_t should_launch = (boot_mode & local_info->service_id);
        if ((svc_id == SVC_ID_MAX && should_launch) || svc_id == service_id) {

            int32_t local_pid = 0;

            if (local_info->fork_by_init == false) {
                local_pid = leopard->forkServiceLayerByExecName(*local_info);
                local_watchdogtime = SVC_WATCHDOG_TIME;
            } else {
                local_watchdogtime = DISABLE_WATCHDOG_TIME;
            }

            if (add_list) {
                addService(std::string(local_info->service_name), service_id, local_pid,
                    SVC_LIMIT_INTERVAL, SVC_LIMIT_BURST, local_watchdogtime,local_info->fork_by_init, local_info->restart_need_reset, local_info->active, local_info->activeNumber, should_launch, launchLable );
            } else {
                LeopardServiceList* svc = service_find_by_name(std::string(local_info->service_name));

                if (svc != NULL) {
                    svc->pid = local_pid;
                    svc->watchdogtime = local_watchdogtime;
                    svc->lastkicktime = getcurrentruntime();
                    ALOGE("service id %d pid updated pid=%d",service_id,local_pid);
                } else {
                    ALOGE("Can not find service id %d in service list!", service_id);
                }
            }
        }
        fork_service_count++;
    }
    LOGD("added services list size(%d)", service_list.size());
}

/**
*  killApplication
*
* \param[in]    app_id     Application id to be killed
* \return         NONE
* \note    none.
*/
void Leopard::killApplication(const std::string& app)
{
    SYSLOGE("killApplication[%s]", app.c_str());
    /*
     * 1. find app name
     * 2. find pid from /proc/xxx/cmds
     * 3. kill it
     *
     * or
     *
     * 1. save pid when app is created
     * 2. find if pid is exist and correct
     * 3. kill if yes, no go to way one.
     */

    /* TODO: we should make decision which signal we will use to kill it.*/
    LeopardServiceList* svc = service_find_by_name(app);

    if (svc != nullptr) {
        killApplicationByPid(app, svc->pid);
        service_remove(svc);
    }
}

void Leopard::killApplicationByPid(const std::string& app, const pid_t pid)
{
    LOGE("killApplicationByPid E name[%s], pid(%d)", app.c_str(), pid);
    if (pid != 0x00 )
        kill(pid, SIGKILL);
    LOGE("killApplicationByPid X name[%s], pid(%d)", app.c_str(), pid);
}

/**
*  read app_id to fork and perform.
*
* \param[in]    client_id    socket id for client
* \return            NONE
* \note       none.
*/
void Leopard::performRequest(int32_t client_id)
{
    int32_t retVal = 0;
    leopard_msg_t msg;
    memset(&msg, 0x00, sizeof(msg));

    retVal = read(client_id, &msg, sizeof(msg));

    if (retVal >= 0) {
        switch(msg.action) {
            case LEOPARD_ACTION_LAUNCH_APP:
            {
                msg.app[APP_FILE_LENGTH-1] = 0x00;
                forkApplicationByExec(std::string(msg.app), true);
            } break;

            case LEOPARD_ACTION_KILL_APP:
            {
                msg.app[APP_FILE_LENGTH-1] = 0x00;
                killApplication(std::string(msg.app));
            } break;

            case LEOPARD_ACTION_READY_SERVICE:
            {
                msg.app[APP_FILE_LENGTH-1] = 0x00;
                makeServiceReady(std::string(msg.app), msg.pid);
            } break;

            case LEOPARD_PLATFORM_RESET:
            {
                platformReset();
            } break;

            case LEOPARD_WATCHDOG_KICK:
            {
                std::lock_guard<std::mutex> lock(mLock);
                LeopardServiceList* svc = service_find_by_pid(msg.pid);

                if (svc == nullptr) {
                   LOGE("LEOPARD_WATCHDOG_KICK  Can't find (pid:%d)", msg.pid);
                } else {
                   svc->lastkicktime = getcurrentruntime();
                }
            } break;

            case LEOPARD_ACTION_LAUNCH_APP_WITH_LABLE:
            {
                msg.app[APP_FILE_LENGTH-1] = 0x00;
                msg.lable[LABLE_LENGTH-1] = 0x00;
                forkApplicationByExec(std::string(msg.app), true, std::string(msg.lable));
            } break;

        default:
            LOGE("unavailable request($d/%d)", msg.id, msg.action);
            break;
        }
    }else {
        SYSLOGE("Read Fail %d",retVal);
    }
}

/**
*  registerServerSocket.
*
* \param    NONE
* \return       NONE
* \note    create Server socket to wait the request.
*/
void Leopard::registerServerSocket(){
    int32_t retVal = 0;

    server_socket = socket_local_server(BASE_SOCKET_NAME, 0, SOCK_STREAM);
    if (server_socket < 0) {
        SYSLOGE("Can not make Server socket.(%d)", server_socket);
        return;
    }

    retVal = fcntl(server_socket, F_SETFD, FD_CLOEXEC); /**< close-on-exec */
}

/**
*    ClearService by pid, if Pid equal 0 then clear all services.
*
* \param    pid
* \return    NONE
* \note    none.
*/
void Leopard::clearService(pid_t pid)
{
    LOGE("clearService E");

    std::lock_guard<std::mutex> lock(mLock);
    if (service_list.empty()) {
        LOGE("service list is empty!!");
        return;
    }

    std::list<LeopardServiceList*>::iterator service_it;
    for (service_it = service_list.begin(); service_it != service_list.end(); ) {
        if (0x00 == pid || pid == (*service_it)->pid) {
            killApplicationByPid((*service_it)->name, (*service_it)->pid);
            (*service_it)->isReady = 0x00;

            for (int32_t i = 0; i < FAILARRYCOUNT; ++i) {
                (*service_it)->failoccurtime[i] = 0x00;
            }

            if ((*service_it)->service_id == APPID_BASE) {
                delete (*service_it);
                service_it = service_list.erase(service_it);
            } else {
                service_it++;
            }
        }
    }
    LOGE("clearService X");
}
/*
void Leopard::addService(const std::string& name, appid_t service_id, pid_t pid, int16_t StartLimitInterval,
        int16_t StartLimitBurst, int watchdogtime, bool fork_by_init, bool restart_need_reset, const std::string& lable)
{
    LeopardServiceList* svc = new LeopardServiceList();

    svc->name = name;
    svc->service_id = service_id;
    svc->isReady = 0;
    svc->pid = pid;
    svc->StartLimitInterval = StartLimitInterval;
    svc->StartLimitBurst = StartLimitBurst;
    svc->watchdogtime = watchdogtime;
    svc->lastkicktime = getcurrentruntime();
    svc->restart_need_reset = restart_need_reset;
    svc->fork_by_init = fork_by_init;
    svc->lable = lable;

    LOGI("addService[%s], id(%d), in_pid(%d), pid(%d), lable(%s)", svc->name.c_str(), svc->service_id, pid, svc->pid, svc->lable.c_str());
    service_add(svc);
}
*/
void Leopard::addService(const std::string& name, appid_t service_id, pid_t pid, int16_t StartLimitInterval,
        int16_t StartLimitBurst, int watchdogtime, bool fork_by_init, bool restart_need_reset,  bool Active, int16_t ActiveNumber, int32_t Boot_Reason, const std::string& lable )
{
    LeopardServiceList* svc = new LeopardServiceList();

    svc->name = name;
    svc->service_id = service_id;
    svc->isReady = 0;
    svc->pid = pid;
    svc->StartLimitInterval = StartLimitInterval;
    svc->StartLimitBurst = StartLimitBurst;
    svc->watchdogtime = watchdogtime;
    svc->lastkicktime = getcurrentruntime();
    svc->restart_need_reset = restart_need_reset;
    svc->fork_by_init = fork_by_init;
    svc->lable = lable;

    svc->active = Active;
    svc->activeNumber = ActiveNumber;
    svc->boot_Reason = Boot_Reason;

    LOGI("addService[%s], id(%d), in_pid(%d), pid(%d), lable(%s), active(%d), activeNumber(%d), boot_Reason(%d)", svc->name.c_str(), svc->service_id, pid, svc->pid, svc->lable.c_str(), svc->active,svc->activeNumber, svc->boot_Reason);
    service_add(svc);
}

void Leopard::service_add(LeopardServiceList* new_service)
{
    std::lock_guard<std::mutex> lock(mLock);
    service_list.push_back(new_service);
}

void Leopard::service_remove(const LeopardServiceList* new_service)
{
    std::lock_guard<std::mutex> lock(mLock);
    std::list<LeopardServiceList*>::iterator service_it;
    for (service_it = service_list.begin(); service_it != service_list.end(); ) {
        if (new_service == (*service_it)) {
            LOGD("remove name[%s] appid(%d)", new_service->name.c_str(), new_service->service_id);
            delete (*service_it);
            service_it = service_list.erase(service_it);
        } else {
            service_it++;
        }
    }

}

int32_t Leopard::set_service_ready(const std::string& name, pid_t pid) {
    LeopardServiceList* svc = service_find_by_name(name);

    if (svc == nullptr) {
       // LOGE("Can not get service list!!");
        return -1;
    }

    if (svc->isReady == 0 && svc->activeNumber == Level_1) {
        svc->isReady = 1;
        readies++;
        LOGI("set_service_ready %s : readies %d" ,svc->name.c_str(), readies );
        svc->active = true;
    }
    svc->pid = pid;

    return readies;
}

int32_t Leopard::set_service_ready_lev2(const std::string& name, pid_t pid) {
    LeopardServiceList* svc = service_find_by_name(name);

    if (svc == nullptr) {
        //LOGE("Can not get service list!!");
        return -1;
    }

    if (svc->isReady == 0 && svc->activeNumber == Level_2 ) {
        svc->isReady = 1;

        readies_lev2++;
        LOGI("set_service_ready_lev2 readies_lev2 %d" , readies_lev2 );
        LOGI("set_service_ready_lev2 %s : readies_lev2 %d" ,svc->name.c_str(), readies_lev2 );
        svc->active = true;

    }
    svc->pid = pid;

    return readies_lev2;
}



void Leopard::set_ready_complete(int32_t ready) {
    ready_complete = ready;

    if (ready_complete == 0x00) {
        readies =0;
    }
}

void Leopard::set_ready_complete_lev2(int32_t ready) {
    ready_complete_lev2 = ready;

    if (ready_complete_lev2 == 0x00) {
        readies_lev2 =0;
    }
}

int32_t Leopard::get_ready_complete(void) {
    return ready_complete;
}

int32_t Leopard::get_ready_complete_lev2(void) {
    return ready_complete_lev2;
}


LeopardServiceList* Leopard::service_find_by_name(const std::string& name)
{
    std::lock_guard<std::mutex> lock(mLock);
    std::list<LeopardServiceList*>::iterator service_it;
    for (service_it = service_list.begin(); service_it != service_list.end(); ++service_it) {
        if (name == (*service_it)->name) {
            return (*service_it);
        }
    }

    return nullptr;
}


bool Leopard::getServiceActive_find_by_name(const std::string& name)
{
    std::lock_guard<std::mutex> lock(mLock);
    std::list<LeopardServiceList*>::iterator service_it;
    for (service_it = service_list.begin(); service_it != service_list.end(); ++service_it) {
        if (name == (*service_it)->name) {
            return (*service_it)->active;
        }
    }
    return false;
}

bool Leopard::getNot_BootComleteList(char* startName)
{
    std::string Name = "";
       int32_t cbit;
       
       switch(boot_mode) {
           case BOOT_MODE_FACTORY:
               cbit = LAUNCH_FACTORY;
               LOGI("bootmode : BOOT_MODE_FACTORY");
               break;
           case BOOT_MODE_DOWNLOAD:
               cbit = LAUNCH_DOWNLOAD;
               LOGI("bootmode : BOOT_MODE_DOWNLOAD");
               break;
           case BOOT_MODE_DEVELOPER:
               cbit = LAUNCH_DEVELOPER;
               LOGI("bootmode : BOOT_MODE_DEVELOPER");
               break;
           case BOOT_MODE_NADTEST:
               cbit = LAUNCH_NADTEST;
               LOGI("bootmode : BOOT_MODE_NADTEST");
               break;
           case BOOT_MODE_FACTORY_RESET:
               cbit = LAUNCH_FACTORY_RESET;
               LOGI("bootmode : BOOT_MODE_FACTORY_RESET");
               break;
           case BOOT_MODE_NORMAL:
                LOGI("bootmode : BOOT_MODE_NORMAL");
           default:
               cbit = LAUNCH_NORMAL;
               break;
       }
       
    std::lock_guard<std::mutex> lock(mLock);
    std::list<LeopardServiceList*>::iterator service_it;

    
    for (service_it = service_list.begin(); service_it != service_list.end(); ++service_it) {

       if ((*service_it)->boot_Reason) {            
            if ((*service_it)->activeNumber == Level_1) {
                if ((*service_it)->active == false) {
                    Name = (*service_it)->name;
                    LOGE("%s Not_bootComlete Level 1 Service [%s])", startName, (*service_it)->name.c_str());
                }
                else 
                    LOGI("%s bootComlete Level 1 Service [%s])", startName, (*service_it)->name.c_str());
            } else if ((*service_it)->activeNumber ==Level_2) {
                if ((*service_it)->active ==false) {
                    Name = (*service_it)->name;
                    LOGE("%s Not_bootComlete Level 2 Service [%s])", startName, (*service_it)->name.c_str());
                } else 
                    LOGI("%s bootComlete Level 2 Service [%s])", startName, (*service_it)->name.c_str());
            }
            else {
            }; 
       }
        else
            { 
        }

    }
    return false;
}

int16_t Leopard::getServiceActiveNumber_find_by_name(const std::string& name)
{
    std::lock_guard<std::mutex> lock(mLock);
    std::list<LeopardServiceList*>::iterator service_it;
    for (service_it = service_list.begin(); service_it != service_list.end(); ++service_it) {
        if (name == (*service_it)->name) {
            return (*service_it)->activeNumber;
        }
    }
    return 0;
}

bool Leopard::setServiceActive_find_by_name(const std::string& name)
{
    std::lock_guard<std::mutex> lock(mLock);
    std::list<LeopardServiceList*>::iterator service_it;
    for (service_it = service_list.begin(); service_it != service_list.end(); ++service_it) {
        if (name == (*service_it)->name) {
            (*service_it)->active = true;
            return true;
        }
    }
    return false;
}

LeopardServiceList* Leopard::service_find_by_appid(int32_t id)
{
    std::lock_guard<std::mutex> lock(mLock);
    std::list<LeopardServiceList*>::iterator service_it;
    for (service_it = service_list.begin(); service_it != service_list.end(); ++service_it) {
        if (id == (*service_it)->service_id) {
            return (*service_it);
        }
    }

    return nullptr;
}

LeopardServiceList* Leopard::service_find_by_pid(pid_t pid)
{
    std::list<LeopardServiceList*>::iterator service_it;
    for (service_it = service_list.begin(); service_it != service_list.end(); ++service_it) {
        if (pid == (*service_it)->pid) {
            return (*service_it);
        }
    }

    return nullptr;
}

void Leopard::makeServiceReady(const std::string& service_name, pid_t pid) {
    std::string service_path("/usr/bin/");
    service_path.append(service_name);

    int32_t tiger_readies = set_service_ready(service_path, pid);
    int32_t tiger_readies_lev2 = set_service_ready_lev2(service_path, pid);
    //uint32_t size = sizeof(service_process_info) / sizeof(LeopardServiceInfo) - 1;
    //int32_t svc_n = count_services_should_launched(boot_mode);
    int32_t svc_n = count_services_should_launched_lev1(boot_mode);
    int32_t svc_n_lev2 = count_services_should_launched_lev2(boot_mode);

    if (getServiceActiveNumber_find_by_name(service_path) == Level_1)
    {
        LOGV("makeServiceReady(), app[%s], readies(%d), info(%d) pid(%d)",
        service_path.c_str(), readies, svc_n, pid);
    } else if(getServiceActiveNumber_find_by_name(service_path) == Level_2)
    {
        LOGV("makeServiceReady(), app[%s], readies_lev2(%d), info(%d) pid(%d)",
        service_path.c_str(), readies_lev2, svc_n_lev2, pid);
    } else {
        LOGV("makeServiceReady(), app[%s], readies_no_level(%d), info(%d) pid(%d)",
        service_path.c_str(), readies, svc_n_lev2, pid);
    }

    if ( getServiceActiveNumber_find_by_name(service_path) == Level_1)
    {
        if (get_ready_complete() == 0 && readies == launch_service_n) {
            LOGI("Boot Complete level 1 ");
            getNot_BootComleteList("LEV1_OK");
            set_ready_complete(1/*true*/);
            sp<IApplicationManagerService> svc = interface_cast<IApplicationManagerService>
            (defaultServiceManager()->getService(String16(APPLICATION_SRV_NAME)));
            //svc->setBootCompleted(true);
            svc->enumerateAvailableApplication();
            leopard->initTimer();
    } else if (get_ready_complete() == 1) {
        LeopardServiceList* svc = service_find_by_name(service_path);

        if (svc == nullptr) {
            LOGE("Can not get service list!!");
            return ;
        }

        if(std::string("ApplicationManagerService") == service_name) {
            LOGD("appmanager reset [%s]", service_name.c_str());
            resetAllApplications();
        }

        // watchdog count here !
        if (svc->fork_by_init == true) {
            LOGV("It was fork by init app[%s], readies(%d), info(%d) pid(%d) and restarted, Count watchdog!!",
            service_path.c_str(), readies, svc_n, pid);
            checkSystemWatchdogBursted(*svc);
         }
        } else {
            /* DO NOTHING */
        }

    }else if ( getServiceActiveNumber_find_by_name(service_path) == Level_2 )
    {
        if (get_ready_complete_lev2() == 0 && readies_lev2== launch_service_n_lev2) {

            LOGI("Boot Complete level 2 ");
            getNot_BootComleteList("LEV2_OK");
            set_ready_complete_lev2(1/*true*/);

            sp<IApplicationManagerService> svc = interface_cast<IApplicationManagerService>
            (defaultServiceManager()->getService(String16(APPLICATION_SRV_NAME)));

            svc->setBootCompleted_Lev2(1/*true*/);

   } else if (get_ready_complete_lev2() == 1) {
        LeopardServiceList* svc = service_find_by_name(service_path);
        if (svc == nullptr) {
            LOGE("Can not get service list!!");
            return ;
        }

        if(std::string("ApplicationManagerService") == service_name) {
            LOGD("appmanager reset [%s]", service_name.c_str());
            resetAllApplications();
        }

        // watchdog count here !
        if (svc->fork_by_init == true) {
            LOGI("It was fork by init app[%s], readies(%d), info(%d) pid(%d) and restarted, Count watchdog!!",
            service_path.c_str(), readies, svc_n, pid);
           checkSystemWatchdogBursted(*svc);
        }
        } else {
        /* DO NOTHING */
        }
    }else {
    }
}

void Leopard::dumpService()
{
    std::lock_guard<std::mutex> lock(mLock);
    if (0 == service_list.size()) {
        return;
    }
    LOGE("########  dumpService");
    std::list<LeopardServiceList*>::iterator service_it;
    for (service_it = service_list.begin(); service_it != service_list.end(); ++service_it) {
        SYSLOGI("dumpService name[%s] id(%d) pid(%d)",
            (*service_it)->name.c_str(), (*service_it)->service_id, (*service_it)->pid);
    }
    LOGE("########  finish dumpService");
}

/**
*  Handler for SIGCHLD.
*  It should check SIGCHLD and receive return value when child made some problem as trap \n
*  so that we can avoid to make zombie.
*
* \param[in]   s
* \return        NONE
* \note        signal.h.
*/
void Leopard::sigchildHandler(int32_t s)
{
    pid_t pid;
    int32_t status;

    /* monitor all child process and return now if no exit */
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {

        if (leopard == NULL) {
            SYSLOG_FATAL("start_service but, No leopard instance");
        } else {
            LeopardServiceList* svc = leopard->service_find_by_pid(pid);

            if (svc == nullptr) {
                LOGE("sigchildHandler can not find(pid:%d)", pid);
            } else {
                if (APPID_BASE == svc->service_id) {
                    svc->watchdogtime = APP_WATCHDOG_TIME;
                } else {
                    svc->watchdogtime = SVC_WATCHDOG_TIME;
                }
            }

            /* WIFEXITED returns a nonzero value if the child process terminated normally with exit or _exit. */
            if (WIFEXITED(status)) {
                if (WEXITSTATUS(status) != 0) {
                    LOGE("Process(%d) exit(%d) So, re-start the Process", pid, WEXITSTATUS(status));
                    re_start_service(pid);
                } else {
                    LOGE("Process(%d) exit(%d)", pid, WEXITSTATUS(status));
                }
            } else if (WIFSIGNALED(status)) { /* WIFSIGNALED returns a nonzero value if the child process terminated because it received a signal that was not handled */
                if (WTERMSIG(status) != SIGKILL) {
                    LOGE("Process(%d) was killed by %d", pid, WTERMSIG(status));
                    re_start_service(pid);
                } else {
                    LOGE("Process(%d) met SIGKILL as %d", pid, WTERMSIG(status));
                }
            } else {
                ///nothing.
            }
        }
        /* TODO: if pid is service layer, we should recover that */
    }
}

/**
*    register handler for SIGCHLD to monitor child process.
*
* \param    NONE
* \return       NONE
* \note    none.
*/
void Leopard::registerSigHandler(void)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = sigchildHandler;
    sa.sa_flags = SA_NOCLDSTOP;

    int32_t err = sigaction(SIGCHLD, &sa, NULL);

    if (err < 0) {
        SYSLOGE("Can not register signal handler");
    }
}

/**
*    release handler for SIGCHLD to monitor child process.
*
* \param    NONE
* \return       NONE
* \note    none.
*/
void Leopard::releaseSigHandler(void)
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));

    sa.sa_handler = SIG_DFL;

    int32_t err = sigaction(SIGCHLD, &sa, NULL);

    if (err < 0) {
        SYSLOGE("Can not register signal handler");
    }
}

void Leopard::initNotBootTimer()
{
    mBootTimerImp = new BootTimeoutHandlerImp (*this);
    mNotBootTimer = new Timer(mBootTimerImp, TIMER_ID_NOT_BOOTCOMPLETEPRELIST);
    mNotBootTimer->setDuration(CHECK_NOT_BOOTCOMPLETEPRE_TIME,CHECK_NOT_BOOTCOMPLETEPRE_TIME);
    mNotBootTimer->start();
}

void Leopard::initBootTimer()
{
    mBootTimer = new Timer(mBootTimerImp, TIMER_ID_FORCE_BOOTCOMPLETEPRE);
    mBootTimer->setDuration(CHECK_BOOTCOMPLETEPRE_TIMEOUT,CHECK_BOOTCOMPLETEPRE_TIMEOUT);
    mBootTimer->start();
}

void Leopard::initLogTimer()
{
    mLogTimer = new Timer(mBootTimerImp, TIMER_ID_REMIND_BOOTCOMPLETEPRE);
    mLogTimer->setDuration(CHECK_BOOTCOMPLETEPRE_TIMEOUT,CHECK_BOOTCOMPLETEPRE_TIMEOUT);
    mLogTimer->start();
	getNot_BootComleteList("NOK after 30 Sec");

}

void Leopard::notBootCompleteforRightTime()
{
	getNot_BootComleteList("10sec after boot-up");
	mNotBootTimer->stop();
	leopard->initBootTimer();
}


void Leopard::forceBootCompletePre()
{
    if(get_ready_complete() == 0) {

        LOGE("Detects Boot Complete pre stuck %d seconds after booting", (CHECK_BOOTCOMPLETEPRE_TIMEOUT+CHECK_NOT_BOOTCOMPLETEPRE_TIME));

		getNot_BootComleteList("ERR");

        sp<IApplicationManagerService> svc = interface_cast<IApplicationManagerService>
            (defaultServiceManager()->getService(String16(APPLICATION_SRV_NAME)));

        LOGI("Force Boot Complete Pre !");
        sp<Post> post = Post::obtain(BOOT_COMPLETE_PRE);
        svc->broadcastSystemPost(post);

        mBootTimer->stop();
        leopard->initLogTimer();
    }
    else {
        mBootTimer->stop();
    }
}

void Leopard::logBootCompletePre()
{
    LOGE("This booting was forced to boot complete pre");
}

// not used
/*
void Leopard::forkDaemonByExec(bool add_list, int dm_id)
{
    ALOGE("start to fork servicelayer daemon is_add_list(%d) daemon_cnt(%d)", add_list, dm_id);
    int fork_daemon_count = 0;

    registerSigHandler();

    while (daemon_process_info[fork_daemon_count].daemon_name != NULL) {
        LeopardDaemonInfo *local_info = &(daemon_process_info[fork_daemon_count]);

        int32_t daemon_id = (~LAUNCH_BIT_ALL & local_info->daemon_id);
        int32_t should_launch = (boot_mode & local_info->daemon_id);

        if ((dm_id == DAEMON_ID_MAX && should_launch) || dm_id == daemon_id) {

            int32_t local_pid = 0;

            if (local_info->fork_by_init == false) {
                local_pid = leopard->forkDaemonByExecName(*local_info);
            }

            if(add_list) {
                addSlDaemon(std::string(local_info->daemon_name), daemon_id, local_pid);
            } else {
                LeopardDaemonList* daemon = sldaemon_find_by_daemonid(daemon_id);

                if (daemon != NULL) {
                    daemon->pid = local_pid;
                }
            }
        }
        fork_daemon_count++;
    }
}

int32_t Leopard::forkDaemonByExecName(const LeopardDaemonInfo& target_daemon)
{
    LOGV("forkDaemonByExecName %s", target_daemon.daemon_name);
    pid_t pid = fork();

    if(pid == 0) {
        if((smack_check() == 0) && (set_label("_") != 0)){
            _exit(0);
        }

        int32_t args_size = 5U;
        char **args = new char*[args_size]; // fixed misra c++ 5-2-8

        for (int32_t i = 0; i < args_size; i++) {
            args[i] = new char[ADDSERVICE_NAME_MAX];
            memset(args[i], 0, sizeof(char)*ADDSERVICE_NAME_MAX);
        }

        strncpy(args[0], target_daemon.daemon_name, ADDSERVICE_NAME_MAX-1); // fixed misra c++ 5-2-5
        strncpy(args[1], target_daemon.arg[0], ADDSERVICE_NAME_MAX-1);      // fixed misra c++ 5-2-5
        strncpy(args[2], target_daemon.arg[1], ADDSERVICE_NAME_MAX-1);      // fixed misra c++ 5-2-5
        strncpy(args[3], target_daemon.arg[2], ADDSERVICE_NAME_MAX-1);      // fixed misra c++ 5-2-5
        args[4] = NULL;

        if (execv(args[0], args)) {
            ALOGE("execv forkDaemon failed (%s)", strerror(errno));
            ALOGE("args[%s], args[%s], args[%s], args[%s], args[%s]", args[0], args[1], args[2], args[3], args[4]);
        }

        ALOGE("Should never get here as creating Daemon");

        for (int32_t i = 0; i < args_size; i++) {
            delete [] args[i];
        }
        delete [] args;

        _exit(0);
    }

    return pid;
}

void Leopard::addSlDaemon(const std::string& name, appid_t id, pid_t pid)
{
    LeopardDaemonList* daemon = new LeopardDaemonList();

    daemon->name = name;
    daemon->daemon_id = id;
    daemon->pid = pid;
    LOGI("addDaemon[%s], id(%d), pid(%d)", daemon->name.c_str(), daemon->daemon_id, daemon->pid);

    sldaemon_add(daemon);
}

void Leopard::sldaemon_add(   LeopardDaemonList* new_daemon)
{
    daemon_list.push_back(new_daemon);
}

LeopardDaemonList* Leopard::sldaemon_find_by_daemonid(int32_t id)
{
    std::list<LeopardDaemonList*>::iterator daemon_it;
    for (daemon_it = daemon_list.begin(); daemon_it != daemon_list.end(); ++daemon_it) {
        if (id == (*daemon_it)->daemon_id) {
            return (*daemon_it);
        }
    }

    return nullptr;
}
*/
