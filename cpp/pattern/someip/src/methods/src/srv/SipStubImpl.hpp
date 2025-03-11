// Copyright (C) 2014-2019 Bayerische Motoren Werke Aktiengesellschaft (BMW AG)
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef myInterfaceSTUBIMPL_HPP_
#define myInterfaceSTUBIMPL_HPP_

#include <CommonAPI/CommonAPI.hpp>
#include <v1/taikt/myInterfaceStubDefault.hpp>

#include "Promise.h"
#include "Dispatcher.h"
#include "Message.h"
using namespace kt;

class ResponseMsg;
class RequestManager;

class myInterfaceStubImpl: public v1_2::taikt::myInterfaceStubDefault {

public:
    myInterfaceStubImpl(std::shared_ptr<Dispatcher>& mExecutor_);
    virtual ~myInterfaceStubImpl();
    virtual void incCounter();
    virtual void foo(const std::shared_ptr<CommonAPI::ClientId> _client,
            int32_t _x1, std::string _x2,
            fooReply_t _reply);
private:
    int cnt;

    std::shared_ptr<Dispatcher> m_Executor;
    Promise<ResponseMsg> mytestpromise;
};

#endif // myInterfaceSTUBIMPL_HPP_
