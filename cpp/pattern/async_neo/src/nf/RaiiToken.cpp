/*
 * BMW Neo Framework
 *
 * Copyright (C) 2021-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 *     Szymon Wojtczak <szymon.wojtczak@globallogic.com>
 */

#include "nf/RaiiToken.h"

#include <nf/Logging.h>

#include <stdexcept>

using namespace nf;

RaiiToken::RaiiToken() noexcept = default;

RaiiToken::RaiiToken(std::function<void()> &&action, Behavior behaviour) noexcept
    : m_action(std::move(action))
    , m_behavior(behaviour)
{
}

RaiiToken::RaiiToken(RaiiToken &&rhs) noexcept
    : m_action(std::move(rhs.m_action))
    , m_behavior(rhs.m_behavior)
{
    rhs.m_action = nullptr;
}

RaiiToken::~RaiiToken()
{
    reset();
}

RaiiToken &RaiiToken::operator=(RaiiToken &&rhs) noexcept
{
    reset();
    m_action = std::move(rhs.m_action);
    m_behavior = rhs.m_behavior;
    rhs.m_action = nullptr;
    return *this;
}

void RaiiToken::reset() noexcept
{
    if (m_action) {
        try {
            m_action();
        } catch (...) {
            nf::fatal("RAII token action has thrown an exception! Terminating...");
            std::terminate();
        }
        m_action = nullptr;
    }
}

RaiiToken RaiiToken::dismissible(std::function<void()> &&action) noexcept
{
    return RaiiToken{std::move(action), Behavior::Dismissible};
}

RaiiToken RaiiToken::nonDismissible(std::function<void()> &&action) noexcept
{
    return RaiiToken{std::move(action), Behavior::NonDismissible};
}

void RaiiToken::dismiss()
{
    if (m_behavior == Behavior::NonDismissible) {
        throw std::runtime_error("Cannot dismiss a non-dismissible RAII token");
    }
    m_action = nullptr;
}
