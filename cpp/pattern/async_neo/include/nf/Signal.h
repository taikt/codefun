/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019-2021 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 *     Alexey Timofeyev <alexey.timofeyev@partner.bmw.de>
 */

#pragma once

#include "detail/SignalBase.h"

#include <nf/Assert.h>
#include <nf/Subscription.h>

#include <type_traits>

NF_BEGIN_NAMESPACE

class Context;

/**
 * @ingroup nf_core_Signals
 * @brief The signal.
 *
 * @code
 * #include <nf/Signal.h>
 * @endcode
 *
 * This class implements a signal, which can be subscribed to and which can be notified
 * (triggered). See @ref nf_core_Signals for more details.
 */
template <typename... tArgs>
class Signal : public detail::SignalNtBase
{
public:
    /**
     * @brief Subscribe to the signal (synchronous).
     * @param f Function to be called when the signal is emitted.
     * @returns Subscription RAII token.
     * @see @ref nf_sig_SyncSub
     */
    template <typename tFunc>
    [[nodiscard]] Subscription subscribe(tFunc &&f) const noexcept
    {
        static_assert(std::is_invocable_v<tFunc, tArgs...>,
                      "Provided callable is not compatible with signal arguments");

        using Listener = detail::DirectListener<tFunc, tArgs...>;
        return bind(std::make_shared<Listener>(std::forward<tFunc>(f)));
    }

    /**
     * @brief Subscribe to the signal (asynchronous).
     * @param executor Executor to post @p f to.
     * @param f Function to be invoked through the executor when the signal is emitted.
     * @returns Subscription RAII token.
     * @see @ref nf_sig_AsyncSub
     */
    template <typename tFunc>
    [[nodiscard]] Subscription subscribe(const std::shared_ptr<Executor> &executor,
                                         tFunc &&f) const noexcept
    {
        static_assert(std::is_invocable_v<tFunc, tArgs...>,
                      "Provided callable is not compatible with signal arguments");

        assertThat(executor != nullptr, "Executor must not be null");

        using Listener = detail::QueuedListener<tFunc, tArgs...>;
        return bind(std::make_shared<Listener>(executor, std::forward<tFunc>(f)));
    }

    /**
     * @brief Subscribe to the signal (with context).
     *
     * @note This is an asynchronous subscription; notifications are posted to the executor
     * associated with the given context, rather than invoked directly.
     *
     * @param context Context to bind the subscription to.
     * @param f Function to be invoked through the context's executor when the signal is emitted.
     * @see @ref nf_sig_CtxSub
     */
    template <typename tFunc>
    void subscribe(const Context &context, tFunc &&f) const noexcept
    {
        static_assert(std::is_invocable_v<tFunc, tArgs...>,
                      "Provided callable is not compatible with signal arguments");

        using Listener = detail::ContextualListener<tFunc, tArgs...>;
        bind(context, std::make_shared<Listener>(std::forward<tFunc>(f)));
    }

    /**
     * @brief Notify the signal.
     *
     * Notifies the listeners subscribed to the signal with the given arguments.
     *
     * @param args Arguments for notification.
     */
    void notify(const tArgs &...args) const noexcept
    {
        /* Notify on a copy of listeners because the list can be modified in process. */
        std::unique_lock lock(m_mutex);
        auto listeners = this->m_listeners;
        lock.unlock();

        for (const auto &wListener : listeners) {
            if (auto listener = wListener.lock()) {
                std::static_pointer_cast<detail::ListenerBase<tArgs...>>(listener)->invoke(args...);
            }
        }
    }
};

NF_END_NAMESPACE
