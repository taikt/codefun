/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include "nf/Function.h"

#include <nf/Logging.h>

#include <functional>

using namespace nf::detail;

void FunctionNtBase::terminateOnEmptyTarget() noexcept
{
    nf::fatal("An empty nf::Function is invoked! Terminating...");
    std::terminate();
}

void FunctionNtBase::throwOnEmptyTarget()
{
    nf::fatal("An empty nf::Function is invoked! Throwing...");
    throw std::bad_function_call();
}
