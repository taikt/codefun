/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019-2021 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include "nf/detail/SignalBase.h"

#include <nf/Context.h>
#include <nf/Logging.h>
#include <nf/Subscription.h>

using namespace nf;
using namespace nf::detail;

SignalNtBase::~SignalNtBase()
{
    for (const auto &wListener : m_listeners) {
        if (auto listener = wListener.lock()) {
            listener->m_signal = nullptr;
            listener->decontextualize();
        }
    }
    m_listeners.clear();
}

Subscription SignalNtBase::bind(std::shared_ptr<ListenerNtBase> &&listener) const noexcept
{
    std::unique_lock lock(m_mutex);
    listener->m_it = m_listeners.insert(end(m_listeners), listener);
    listener->m_signal = this;
    return Subscription{std::move(listener)};
}

void SignalNtBase::bind(const Context &context,
                        std::shared_ptr<ListenerNtBase> &&listener) const noexcept
{
    std::unique_lock lock(m_mutex);
    listener->m_it = m_listeners.insert(end(m_listeners), listener);
    listener->m_signal = this;
    context.bind(std::move(listener));
}

void SignalNtBase::unbind(ListenerNtBase &listener) const noexcept
{
    std::unique_lock lock(m_mutex);
    listener.m_signal = nullptr;
    m_listeners.erase(listener.m_it);
}

ListenerNtBase::ListenerNtBase() noexcept = default;

ListenerNtBase::~ListenerNtBase()
{
    if (m_signal) {
        m_signal->unbind(*this);
    }
}
