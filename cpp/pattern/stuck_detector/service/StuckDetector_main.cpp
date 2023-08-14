

/**
 * @attention Copyright (c) 2019 by LG electronics co, Ltd. All rights reserved.
 *   This program or software including the accompanying associated documentation ("Software") is
 *   the proprietary software of LG Electronics Inc.  and or its licensors, and may only be used,
 *   duplicated, modified or distributed pursuant to the terms and conditions of a separate written license agreement
 *   between you and LG Electronics Inc. ("Authorized License").
 *   Except as set forth in an Authorized License, LG Electronics Inc. grants no license (express or implied), rights to use,
 *   or waiver of any kind with respect to the Software, and LG Electronics Inc. expressly reserves all rights in
 *   and to the Software and all intellectual property therein.
 *   If you have no Authorized License, then you have no rights to use the Software in any ways,
 *   and should immediately notify LG Electronics Inc. and discontinue all use of the Software.
 *
 * @author  tai2.tran@lge.com

 * @version
 *  1.3  add Low Level Design Documents (This is doxygen comments)
 */

#include <unistd.h>

#include "Log.h"
#include <dlt/dlt.h>
#include <pthread.h>
#include <systemd/sd-daemon.h>

#include <time.h>
#include "basic_types.h"
#include <iostream>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "Reactor.h"
#include "AcceptorHandler.h"
#include "ReaderHandler.h"

#include "SocketException.h"
#include "StuckDetectorManager.h"
#include "connectionManager.h"


#define LOG_TAG "stuckdetectorMain"


void *watcher_thread (void *vargp)
{
    do {

        sd_notify(0, "WATCHDOG=1");


        int sleep_sec = 0;
        do {
            sleep(5);
            sleep_sec += 5;
        } while(sleep_sec < 15);

    } while(1);
}

using namespace std;


void *runReactorthread (void *vargp)
{

	LOGI("register accept handler ");

	StuckDetectorManager::getInstance()->onInit();
    Reactor::getInstance()->registerEventHandler(ACCEPT_EVENT,AcceptorHandler::getInstance());
	LOGI("register read handler ");
	Reactor::getInstance()->registerEventHandler(READ_EVENT,ReaderHandler::getInstance());
	LOGI("set reactor for accept handler ");
	AcceptorHandler::getInstance()->setReactor(Reactor::getInstance());
	LOGI("set reactor for read handler  ");
	ReaderHandler::getInstance()->setReactor(Reactor::getInstance());
	LOGI("done register ");

	try {
		 LOGI("init reactor ");
		 Reactor::getInstance()->initialize();
		 LOGI("done init reactor ");
	} catch (SocketException &e) {
		cout<<"Exception appeared during dispatcher initialization: "<< e.what()<<endl;
		cout<<"No need to continue terminate program"<<endl;
		 Reactor::getInstance()->deinitialize();
		exit(1);
	} catch(...)
	{
		LOGI("exception when init reactor");
		cout << "Cought exception"<<endl;
		//exit(1);
	}

	Reactor::getInstance()->startRunning();
/*
    boost::asio::io_service io_service;
    unlink("/dev/stuckdetector_socket");

    Server server(io_service,"/dev/stuckdetector_socket");
    io_service.run();
*/
	return;

}

DLT_DECLARE_CONTEXT(stuckdetectorctx);

#ifdef __UNITTEST__
int UT_main(int argc, char **argv)
#else
int main(int argc, char **argv)
#endif
{
    //DLT_REGISTER_APP("SD", "StuckDetector");
    //DLT_REGISTER_CONTEXT(stuckdetectorctx, "SD", "StuckDetector");
    __init_log_id("StuckDetector");
    pthread_t tid;
    int err = pthread_create (&tid, NULL, watcher_thread, NULL);

    sd_notify(0, "READY=1");

	pthread_t reactor_thread;
    int err2 = pthread_create (&reactor_thread, NULL, runReactorthread, NULL);
	pthread_join(reactor_thread, NULL);

 	/*
		while(1) {
		sleep(3);
		LOGI("what happend");
	}
	*/

	//start_message_dispoatcher();

    //DLT_UNREGISTER_CONTEXT(stuckdetectorctx);
    //DLT_UNREGISTER_APP();

    return 0;
}
