/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#pragma once

#include <nf/ContextItem.h>
#include <nf/Executor.h>

#include <boost/core/noncopyable.hpp>

#include <list>
#include <memory>
#include <mutex>

NF_BEGIN_NAMESPACE

class Subscription;

namespace detail {

class ListenerNtBase;

class SignalNtBase : private boost::noncopyable
{
    friend class ListenerNtBase;

public:
    using ListenerList = std::list<std::weak_ptr<ListenerNtBase>>;
    using Iterator = ListenerList::iterator;

public:
    // DEPRECATED. Move it to protected after C++20.
    ~SignalNtBase();

protected:
    Subscription bind(std::shared_ptr<ListenerNtBase> &&listener) const noexcept;
    void bind(const Context &context, std::shared_ptr<ListenerNtBase> &&listener) const noexcept;
    void unbind(ListenerNtBase &listener) const noexcept;

protected:
    mutable std::mutex m_mutex;
    mutable ListenerList m_listeners;
};

class ListenerNtBase : public ContextItem
{
    friend SignalNtBase;
    friend Subscription;

protected:
    ListenerNtBase() noexcept;
    ~ListenerNtBase() override;

private:
    SignalNtBase::Iterator m_it;
    const SignalNtBase *m_signal{nullptr};
};

template <typename... tArgs>
class ListenerBase : public ListenerNtBase
{
public:
    virtual void invoke(const tArgs &...args) noexcept = 0;
};

template <typename tFunc, typename... tArgs>
class DirectListener final : public ListenerBase<tArgs...>
{
public:
    explicit DirectListener(tFunc &&f) noexcept
        : m_f(std::move(f))
    {
    }

public:
    void invoke(const tArgs &...args) noexcept override
    {
        m_f(args...);
    }

private:
    std::decay_t<tFunc> m_f;
};

template <typename tFunc, typename... tArgs>
class QueuedListener final : public ListenerBase<tArgs...>
{
public:
    QueuedListener(const std::shared_ptr<Executor> &executor, tFunc &&f) noexcept
        : m_executor(executor)
        , m_f(std::move(f))
    {
    }

public:
    void invoke(const tArgs &...args) noexcept override
    {
        auto executor = m_executor.lock();
        if (executor) {
            /* ScopedCallback is not used here for performance reasons. Using it results in
             * one extra move of provided arguments. */
            executor->post(
                [this, wSelf = this->weak_from_this(), args = std::make_tuple(args...)]() mutable {
                    if (auto self = wSelf.lock()) {
                        std::apply(m_f, std::move(args));
                    }
                });
        }
    }

private:
    std::weak_ptr<Executor> m_executor;
    std::decay_t<tFunc> m_f;
};

template <typename tFunc, typename... tArgs>
class ContextualListener final : public ListenerBase<tArgs...>
{
public:
    explicit ContextualListener(tFunc &&f) noexcept
        : m_f(std::move(f))
    {
    }

public:
    void invoke(const tArgs &...args) noexcept override
    {
        /* No guard for this is needed because it guarded by post(). */
        this->post([this, args = std::make_tuple(args...)]() mutable {
            std::apply(m_f, std::move(args));
        });
    }

private:
    std::decay_t<tFunc> m_f;
};

} // namespace detail

NF_END_NAMESPACE
