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

#pragma once

#include "AnyFunction.h"

#include <nf/Executor.h>
#include <nf/Future.h>
#include <nf/Promise.h>

#include <boost/core/noncopyable.hpp>

#include <list>
#include <memory>
#include <mutex>
#include <tuple>
#include <type_traits>

NF_BEGIN_NAMESPACE

class Context;
class ContextItem;
class RaiiToken;
class Subscription;

namespace detail {

// clang-format off
template <typename T>
inline constexpr bool IsNonCallableBindable =
    std::is_same_v<std::remove_reference_t<T>, Subscription>
 || std::is_same_v<std::remove_reference_t<T>, RaiiToken>
 || std::is_convertible_v<T, std::shared_ptr<ContextItem>>;
// clang-format on

// This is a default trait for the `StoredTask::invokeAuto`.
// If you need a different behaviour use template specialization.
template <typename tInvokeResult, typename Enabled = void>
struct InvokeTraits
{
    // Type is used for `nf::Promise<std::optional<UnderlyingType>>`
    using UnderlyingType = tInvokeResult;

    // For the case when `StoredTask::invokeAuto` is called from the same thread
    // as `ContextState` belongs to.
    template <typename tFunc, typename... tArgs>
    static void invoke(Promise<std::optional<UnderlyingType>> &promise, const AnyFunction &task,
                       tArgs &&...args)
    {
        auto result = task.template invoke<tFunc>(std::forward<decltype(args)>(args)...);
        promise.fulfill(result);
    }

    // For the case when `StoredTask::invokeAuto` is invoked from different thread
    // then `ContextState` belongs to.
    template <typename tFunc, typename... tArgs>
    static void apply(Promise<std::optional<UnderlyingType>> &promise, const AnyFunction &task,
                      std::tuple<tArgs...> &&args)
    {
        auto result = task.apply<tFunc>(std::move(args));
        promise.fulfill(result);
    }
};

// Implementation for `void` type
template <>
struct InvokeTraits<void, void>
{
    using UnderlyingType = std::monostate;

    template <typename tFunc, typename... tArgs>
    static void invoke(Promise<std::optional<UnderlyingType>> &promise, const AnyFunction &task,
                       tArgs &&...args)
    {
        task.template invoke<tFunc>(std::forward<decltype(args)>(args)...);
        promise.fulfill(UnderlyingType{});
    }

    template <typename tFunc, typename... tArgs>
    static void apply(Promise<std::optional<UnderlyingType>> &promise, const AnyFunction &task,
                      std::tuple<tArgs...> &&args)
    {
        task.apply<tFunc>(std::move(args));
        promise.fulfill(UnderlyingType{});
    }
};

// Implementation for `nf::Future<Type>` to avoid inception of
// `nf::Future<std::optional<Future<Type>>`
template <typename tInvokeResult>
struct InvokeTraits<tInvokeResult, std::enable_if_t<nf::IsFuture<tInvokeResult>>>
{
    using UnderlyingType = FutureValueType<tInvokeResult>;

    template <typename tFunc, typename... tArgs>
    static void invoke(Promise<std::optional<UnderlyingType>> &promise, const AnyFunction &task,
                       tArgs &&...args)
    {
        auto result = task.template invoke<tFunc>(std::forward<decltype(args)>(args)...);
        auto context = std::make_shared<nf::Context>();
        result.then(*context, [context, promise](auto &&value) {
            promise.fulfill(std::optional<std::remove_reference_t<decltype(value)>>(
                std::forward<decltype(value)>(value)));
        });
    }

    template <typename tFunc, typename... tArgs>
    static void apply(Promise<std::optional<UnderlyingType>> &promise, const AnyFunction &task,
                      std::tuple<tArgs...> &&args)
    {
        auto result = task.apply<tFunc>(std::move(args));
        auto context = std::make_shared<nf::Context>();
        result.then(*context, [context, promise](auto &&value) {
            promise.fulfill(std::optional<std::remove_reference_t<decltype(value)>>(
                std::forward<decltype(value)>(value)));
        });
    }
};

class ContextState final : boost::noncopyable, public std::enable_shared_from_this<ContextState>
{
    friend ContextItem;

public:
    using Task = Function<void()>;
    using TaskList = std::list<AnyFunction>;
    using TaskIterator = TaskList::const_iterator;

    using Item = std::shared_ptr<ContextItem>;
    using ItemList = std::list<Item>;
    using ItemIterator = ItemList::const_iterator;

    using TokenList = std::list<RaiiToken>;

    class StoredTask
    {
    public:
        StoredTask(std::weak_ptr<ContextState> &&state, TaskIterator it) noexcept
            : m_state(std::move(state))
            , m_it(it)
        {
        }

        StoredTask(const StoredTask &) = delete;
        StoredTask(StoredTask &&) noexcept = default;
        StoredTask &operator=(const StoredTask &) = delete;
        StoredTask &operator=(StoredTask &&) noexcept = default;

        ~StoredTask()
        {
            if (auto state = m_state.lock()) {
                // NOTE: actual deletion will happen after unlocking the mutex so that nested tasks
                // can be cleaned up without deadlocking
                [[maybe_unused]] auto delayDelete = *m_it;
                std::unique_lock lock(state->m_mutex);
                state->m_tasks.erase(m_it);
            }
        }

        template <typename tFunc, typename... tArgs>
        auto invokeSync(tArgs &&...args) const
        {
            if (auto state = m_state.lock()) {
                return m_it->template invoke<tFunc>(std::forward<decltype(args)>(args)...);
            }

            /* If the context has been deleted and the callback returns a non-void, we
             * will return a default-constructed value of the given type. */
            return std::invoke_result_t<tFunc, tArgs...>();
        }

        template <typename tFunc, typename... tArgs>
        auto invokeAuto(tArgs &&...args) const
        {
            using InvokeResult = std::invoke_result_t<tFunc, tArgs...>;
            using TraitsType = InvokeTraits<InvokeResult>;

            Promise<std::optional<typename TraitsType::UnderlyingType>> promise;

            if (auto state = m_state.lock()) {
                if (state->m_executor->isThisThread()) {
                    TraitsType::template invoke<tFunc>(promise, *m_it,
                                                       std::forward<tArgs>(args)...);
                } else {
                    auto invokeF = [promise, f = *m_it,
                                    args =
                                        std::make_tuple(std::forward<tArgs>(args)...)]() mutable {
                        TraitsType::template apply<tFunc>(promise, f, std::move(args));
                    };
                    constexpr auto allArgsMovable =
                        (std::is_move_constructible_v<std::decay_t<tArgs>> && ...);
                    if constexpr (allArgsMovable) {
                        state->post(std::move(invokeF));
                    } else {
                        state->post(invokeF);
                    }
                }
            } else {
                promise.fulfill(std::nullopt);
            }
            return promise.future();
        } // NOLINT(clang-analyzer-cplusplus.NewDeleteLeaks)

        template <typename tFunc, typename... tArgs>
        void defer(tArgs &&...args) const
        {
            if (auto state = m_state.lock()) {
                auto invokeF = [f = *m_it,
                                args = std::make_tuple(std::forward<tArgs>(args)...)]() mutable {
                    f.apply<tFunc>(std::move(args));
                };

                constexpr auto allArgsMovable =
                    (std::is_move_constructible_v<std::decay_t<tArgs>> && ...);
                if constexpr (allArgsMovable) {
                    state->post(std::move(invokeF));
                } else {
                    state->post(invokeF);
                }
            }
        }

    private:
        std::weak_ptr<ContextState> m_state;
        TaskIterator m_it;
    };

public:
    static ContextState &from(const Context &context) noexcept;
    static void reset(std::shared_ptr<ContextState> &state) noexcept;

public:
    explicit ContextState(const std::shared_ptr<Executor> &executor) noexcept;
    ~ContextState();

public:
    void post(Task &&task) noexcept;
    void post(Executor::Delay when, Task &&task) noexcept;

    template <typename tFunc, typename = std::enable_if_t<!IsNonCallableBindable<tFunc>>>
    [[nodiscard]] auto bind(tFunc &&f) noexcept
    {
        return [token = store(AnyFunction(std::forward<tFunc>(f)))](auto &&...args) {
            return token->template invokeAuto<tFunc>(std::forward<decltype(args)>(args)...);
        };
    }

    template <typename tFunc>
    [[nodiscard]] auto bindDeferred(tFunc &&f) noexcept
    {
        return [token = store(AnyFunction(std::forward<tFunc>(f)))](auto &&...args) {
            token->template defer<tFunc>(std::forward<decltype(args)>(args)...);
        };
    }

    void bind(Subscription &&sub) noexcept;
    void bind(RaiiToken &&token) noexcept;
    void bind(Item &&item) noexcept;

    bool isEmpty() const noexcept;
    bool isThisThread() const noexcept;

private:
    std::shared_ptr<StoredTask> store(AnyFunction &&task) noexcept;

private:
    mutable std::mutex m_mutex;
    std::shared_ptr<Executor> m_executor;
    TaskList m_tasks;
    ItemList m_items;
    TokenList m_tokens;
};

} // namespace detail

NF_END_NAMESPACE
