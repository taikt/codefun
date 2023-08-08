/*
 * BMW Neo Framework
 *
 * Copyright (C) 2018-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include "nf/detail/CallbackScope.h"

using namespace nf::detail;

CallbackScope::CallbackScope() noexcept
    : m_guard(std::make_shared<Guard>())
{
}

CallbackScope::~CallbackScope() = default;

void CallbackScope::reset() noexcept
{
    m_guard = std::make_shared<Guard>();
}
