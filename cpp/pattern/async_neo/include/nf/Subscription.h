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

#pragma once

#include <nf/Global.h>

#include <boost/core/noncopyable.hpp>

#include <memory>
#include <set>

NF_BEGIN_NAMESPACE

namespace detail {
class ContextState;
class ListenerNtBase;
class SignalNtBase;
} // namespace detail

/**
 * @ingroup nf_core_Signals
 * @brief The subscription.
 *
 * This class holds a subscription to a @nfref{Signal}. See @ref nf_core_Signals
 * for more details.
 */
class Subscription final : boost::noncopyable
{
public:
    Subscription() noexcept;
    Subscription(Subscription &&other) noexcept;
    Subscription &operator=(Subscription &&other) noexcept;

    /// Checks if the subscription is active, i.e. a listener is assigned.
    explicit operator bool() const noexcept;

    void reset() noexcept;

private:
    friend class detail::ContextState;
    friend class detail::SignalNtBase;

    explicit Subscription(std::shared_ptr<detail::ListenerNtBase> &&listener) noexcept;
    void contextualize(detail::ContextState &state) &&noexcept;

private:
    std::shared_ptr<detail::ListenerNtBase> m_listener;
};

NF_END_NAMESPACE
