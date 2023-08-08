/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include "nf/Framework.h"

#include "backend/AsioIoWatch.h"

#include <nf/Assert.h>
#include <nf/ChronoIoStream.h>
#include <nf/Executor.h>
#include <nf/IoWatch.h>
#include <nf/Logging.h>
#include <nf/Singleton.h>
#include <nf/Timer.h>
#include <nf/backend/AsioExecutor.h>

#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>

NF_BEGIN_NAMESPACE

namespace detail {
std::unique_ptr<Framework> g_framework;
} // namespace detail

NF_END_NAMESPACE

using namespace nf;
using namespace detail;

namespace {

/* This is a sequence number of the currently initialized framework. This is needed to allow
 * clients to have different instances of the framework backend during the application's
 * lifetime. The sequence number is used in Executor::thisThread() and mainThread() to
 * determine if the backend has changed and they need to re-construct the executor using the
 * new backend. This is actively used in unit tests where each test case runs with it's own
 * instance of the backend. */
int s_sequenceNr{0}; // NOLINT(readability-identifier-naming)

} // anonymous namespace

void Framework::initialize(std::unique_ptr<Framework> &&framework) noexcept
{
    assertThat(g_framework == nullptr, "Framework is already initialized!");
    g_framework = std::move(framework);
    s_sequenceNr++;

    /* This call is needed to bind the current thread to the main thread. */
    Executor::mainThread();
    nf::info("Framework (seqNr={}) has been initialized!", s_sequenceNr);
}

void Framework::terminate() noexcept
{
    assertThat(g_framework != nullptr, "Framework is not initialized!");
    g_framework.reset();
    nf::info("Framework (seqNr={}) has been terminated!", s_sequenceNr);
    Logging::terminate();
}

int Framework::sequenceNr() noexcept
{
    return s_sequenceNr;
}

std::shared_ptr<Executor> AsioFramework::makeExecutor() noexcept
{
    return std::make_shared<backend::AsioExecutor>();
}
