// Copyright (C) 2014-2019 Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <thread>
#include <iostream>

#include <CommonAPI/CommonAPI.hpp>
#include "SipStubImpl.hpp"

#include "debug.hpp"

#include "Promise.h"
#include "Dispatcher.h"
#include "Message.h"
#include "MsgHandler.h"


int main() {
    CommonAPI::Runtime::setProperty("LogContext", "E03S");
    CommonAPI::Runtime::setProperty("LogApplication", "E03S");
    CommonAPI::Runtime::setProperty("LibraryBase", "myInterface");

    std::shared_ptr<CommonAPI::Runtime> runtime = CommonAPI::Runtime::get();

    std::string domain = "local";
    std::string instance = "MyInstanceTesting"; //commonapi.examples.Mytesting

    std::shared_ptr<Handler> myHandler_ = std::make_shared<MsgHandler>();
    std::shared_ptr<Dispatcher> mExecutor = std::make_shared<Dispatcher>(myHandler_);

    std::shared_ptr<myInterfaceStubImpl> myService = std::make_shared<myInterfaceStubImpl>(mExecutor);
    
    //mExecutor->deliverTask([=]{std::cout<<"[taikt] hello main@@@@@@@@@@@@@@@@@@@@@@@@@\n";});

	
	DEBUG_MSG();
    while (!runtime->registerService(domain, instance, myService, "service-sample")) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    std::cout << "ok, Successfully Registered Service!" << std::endl;

    

    while (true) {
        //myService->incCounter(); // Change value of attribute, see stub implementation
        std::cout << "Waiting for calls... (Abort with CTRL+C)" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(60));
    }

    return 0;
}
