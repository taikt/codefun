/*
 * BMW Neo Framework
 *
 * Copyright (C) 2021-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Alexey Larikov <alexey.larikov@bmw.de>
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#pragma once

#include <nf/Assert.h>
#include <nf/ContextItem.h>
#include <nf/Executor.h>

#include <boost/hana/basic_tuple.hpp>
#include <boost/hana/equal.hpp>
#include <boost/hana/front.hpp>
#include <boost/hana/functional/partial.hpp>
#include <boost/hana/if.hpp>
#include <boost/hana/type.hpp>

#include <functional>
#include <memory>
#include <mutex>
#include <optional>
#include <variant>

NF_BEGIN_NAMESPACE

template <typename tResult>
class Promise;

template <typename tResult>
class Future;

namespace detail {

template <typename tFunc, typename tReturn, typename... tArgument>
class InvocationHelper
{
public:
    InvocationHelper(tFunc f, Promise<tReturn> p) noexcept
        : m_func{std::move(f)}
        , m_promise{std::move(p)}
    {
    }

    explicit InvocationHelper(tFunc &&func) noexcept
        : m_func(std::move(func))
    {
    }

    InvocationHelper(const Context &, tFunc &&func) noexcept
        : m_func(std::move(func))
    {
    }

    auto future() const noexcept
    {
        return m_promise.future();
    }

    static constexpr bool argumentsVoid()
    {
        constexpr auto kArgumentTypes =
            boost::hana::make_basic_tuple(boost::hana::type_c<tArgument>...);

        if constexpr (boost::hana::is_empty(kArgumentTypes)) {
            return true;
        } else {
            return boost::hana::if_(boost::hana::front(kArgumentTypes)
                                        == boost::hana::type_c<std::monostate>,
                                    true, false);
        }
    }

    void operator()([[maybe_unused]] tArgument... value)
    {
        if constexpr (argumentsVoid() && std::is_void_v<tReturn>) {
            m_func();
            m_promise.fulfill();
        } else if constexpr (argumentsVoid() && !std::is_void_v<tReturn>) {
            m_promise.fulfill(m_func());
        } else if constexpr (!argumentsVoid() && std::is_void_v<tReturn>) {
            m_func(std::move(value)...);
            m_promise.fulfill();
        } else {
            m_promise.fulfill(m_func(std::move(value)...));
        }
    }

private:
    std::decay_t<tFunc> m_func;
    Promise<tReturn> m_promise;
};

template <typename tFunc, typename tInnerFuture, typename tReturn>
class InvocationHelper<tFunc, tReturn, Future<tInnerFuture>>
{
public:
    using CallType = std::conditional_t<std::is_void_v<tInnerFuture>, std::monostate, tInnerFuture>;

public:
    InvocationHelper(const Context &context, tFunc &&func) noexcept
        : m_context{context}
        , m_invocator{std::move(func)}
    {
    }

    auto future() const noexcept
    {
        return m_invocator.future();
    }

    void operator()(Future<tInnerFuture> future)
    {
        if constexpr (std::is_void_v<tInnerFuture>) {
            future.then(m_context, [invocator = std::move(m_invocator)]() mutable {
                invocator(std::monostate{});
            });
        } else {
            future.then(m_context,
                        [invocator = std::move(m_invocator)](tInnerFuture value) mutable {
                            invocator(std::move(value));
                        });
        }
    }

private:
    const Context &m_context; // This is safe, because continuation is bound to this context
    InvocationHelper<tFunc, tReturn, CallType> m_invocator;
};

template <typename tValue>
class AsyncSharedState;

template <typename tInput>
class ContinuationBase : public ContextItem
{
public:
    void accept(tInput &&input) noexcept
    {
        /* No guard for this is needed because it guarded by post(). */
        post([this, input = std::move(input)]() mutable {
            handle(std::move(input));
            decontextualize();
        });
    }

    using ContextItem::decontextualize;

private:
    friend AsyncSharedState<tInput>;
    virtual void handle(tInput &&input) = 0;
};

template <typename tFunc, typename tInput, typename tOutput>
class Continuation : public ContinuationBase<tInput>
{
public:
    Continuation(const Context &context, tFunc &&func) noexcept
        : m_invocator(context, std::move(func))
    {
    }

    auto future() noexcept
    {
        return m_invocator.future();
    }

private:
    void handle(tInput &&input) override
    {
        m_invocator(std::move(input));
    }

private:
    InvocationHelper<tFunc, tOutput, tInput> m_invocator;
};

template <typename tValue>
class AsyncSharedState final
{
public:
    AsyncSharedState() = default;
    AsyncSharedState(const AsyncSharedState &) = delete;
    AsyncSharedState(AsyncSharedState &&) = delete;
    AsyncSharedState &operator=(const AsyncSharedState &) = delete;
    AsyncSharedState &operator=(AsyncSharedState &&) = delete;

    ~AsyncSharedState()
    {
        // If a continuation is set and still alive, remove it from context so it gets freed
        if (m_continuation) {
            if (auto ptr = m_continuation->lock()) {
                ptr->decontextualize();
            }
        }
    }

    void setValue(tValue &&value)
    {
        std::unique_lock lock(m_mutex);
        assertThat(!m_value, "Value is already set");
        m_value = std::move(value);
        notify();
    }

    void setContinuation(const Context &context,
                         std::shared_ptr<ContinuationBase<tValue>> &&continuation)
    {
        std::unique_lock lock(m_mutex);
        assertThat(!m_continuation, "Continuation is already set");
        detail::bind(context, continuation);
        m_continuation = std::move(continuation);
        notify();
    }

private:
    void notify() noexcept
    {
        if (m_continuation && m_value) {
            if (auto ptr = m_continuation->lock()) {
                ptr->accept(std::move(*m_value));
            }
            // Invalidate pointer, but keep optional valid to prevent double use
            m_continuation.emplace();
        }
    }

private:
    std::mutex m_mutex;
    std::optional<tValue> m_value;
    std::optional<std::weak_ptr<ContinuationBase<tValue>>> m_continuation;
};

} // namespace detail

NF_END_NAMESPACE
