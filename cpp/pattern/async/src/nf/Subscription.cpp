/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 *     Thomas Leichtle <thomas.leichtle@bmw.de>
 */

#include "nf/Subscription.h"

#include <nf/detail/ContextState.h>
#include <nf/detail/SignalBase.h>

using namespace nf;

Subscription::Subscription() noexcept = default;

Subscription::Subscription(std::shared_ptr<detail::ListenerNtBase> &&listener) noexcept
    : m_listener(std::move(listener))
{
}

Subscription::Subscription(Subscription &&other) noexcept
    : m_listener(std::move(other.m_listener))
{
}

Subscription &Subscription::operator=(Subscription &&other) noexcept
{
    m_listener = std::move(other.m_listener);
    return *this;
}

Subscription::operator bool() const noexcept
{
    return m_listener != nullptr;
}

void Subscription::reset() noexcept
{
    m_listener.reset();
}

void Subscription::contextualize(detail::ContextState &state) &&noexcept
{
    state.bind(std::move(m_listener));
    m_listener.reset();
}
