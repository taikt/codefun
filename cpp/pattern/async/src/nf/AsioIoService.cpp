/*
 * BMW Neo Framework
 *
 * Copyright (C) 2018-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 *     Marko Popović <marko.popovic@bmw.de>
 */
#include "nf/AsioIoService.h"

#include <nf/Assert.h>
#include <nf/Logging.h>
#include <nf/backend/AsioExecutor.h>

using namespace nf;

namespace {

boost::asio::io_service &getAsioService(const std::shared_ptr<Executor> &executor)
{
    auto asioExecutor = std::dynamic_pointer_cast<backend::AsioExecutor>(executor);
    if (!asioExecutor) {
        nf::fatal("Not an ASIO executor");
        std::terminate();
    }
    return asioExecutor->ioService();
}

} // namespace

boost::asio::io_service &AsioIoService::thisThread() noexcept
{
    return getAsioService(Executor::thisThread());
}

boost::asio::io_service &AsioIoService::mainThread() noexcept
{
    return getAsioService(Executor::mainThread());
}
