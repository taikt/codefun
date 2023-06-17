/*
 * BMW Neo Framework
 *
 * Copyright (C) 2021-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#pragma once

#include "Executor.h"
#include "detail/ContextState.h"

#include <memory>

NF_BEGIN_NAMESPACE

/**
 * @ingroup nf_core_ExecutionFlow
 * @brief The execution context.
 *
 * This class represents an execution context, a @e thing which defines @b both a lifetime
 * for various entities, like callables, signal subscriptions or futures, @b and a thread
 * to execute the code in.
 *
 * When created, the context is bound to a specific @ref Executor. By default, this is the
 * executor of the current thread (see @nfrefmethod{Executor,thisThread()}) but a client
 * can specify any other executor as well.
 */
class Context : boost::noncopyable
{
public:
    using Task = detail::ContextState::Task;

public:
    /**
     * @brief Construct a context of a caller's thread.
     *
     * This function constructs an instance of @ref Context and binds it to the executor of
     * the current thread (see @ref Executor::thisThread()).
     */
    Context() noexcept;

    /**
     * @brief Construct a context from a provided executor.
     *
     * This function constructs an instance of @ref Context and binds it to the provided @p
     * executor.
     */
    explicit Context(const std::shared_ptr<Executor> &executor) noexcept;

public:
    /**
     * @brief Post a task to the bound executor.
     *
     * This function enqueues the given @p task to be executed later by the bound executor
     * in a context of its thread.
     *
     * If this instance of @ref Context is @ref reset() "reset" or destroyed before the task
     * is invoked, all associated resources (e.g. captured objects in a lambda) are freed
     * and the task will not be invoked.
     *
     * @see @nfref{Executor::post()}.
     */
    void post(Task &&task) const noexcept;

    /**
     * @brief Post a delayed task to the bound executor.
     *
     * This function enqueues a given task to be executed later (with a given @p delay) by
     * the bound executor.
     *
     * If this instance of @ref Context is @ref reset() "reset" or destroyed before the task
     * is invoked, all associated resources (e.g. captured objects in a lambda) are freed
     * and the task will not be invoked.
     *
     * @see @nfref{Executor::post()}.
     */
    void post(Executor::Delay when, Task &&task) const noexcept;

    /**
     * @brief Bind an arbitrary callable and possibly make it a deferred one.
     *
     * This function binds the provided callable @p f to this context and returns a wrapper
     * callable with the same @e look-and-feel as @p f with the difference of returned value type.
     * The wrapper returns a @ref Future with the @c std::optional.
     *
     * Once the wrapper is invoked the callable @p f is executed in this context if the underlying
     * executor's thread matches this thread. Otherwise the @p f is posted to the underlying
     * executor of its context.
     *
     * If this instance of @ref Context is @ref reset() "reset" or destroyed, the wrapper
     * becomes a no-op function and all associated resources (e.g. captured objects in a
     * lambda) are freed. The no-op function returns a default-constructed value of its
     * return type, unless it is a void function.
     *
     * @note The wrapper returns an object of type @c Future<std::optional<T>> where @c T is a type
     * of object returned by @p f. If the  @c T is @c void then object's type is @c
     * Future<std::monostate>. @c Future<std::nullopt> object is returned if context was deleted.
     *
     * @par Example
     *
     * @code
     * class MyClass
     * {
     * public:
     *     void initializeData()
     *     {
     *         ExternalComponent::getDataAsync(m_context.bind([this](int data)
     *         {
     *             // This callback will be invoked only once and only if "this" is a valid
     *             // pointer, thus it is safe to access m_data. The invocation happens in-place,
     *             // e.g. on the executor or thread on which the caller is on.
     *             m_data = data;
     *         }));
     *     }
     *
     * private:
     *     // Context as a member of a class ensures that all callbacks bound to this context
     *     // will be invalidated when an object of MyClass is destroyed.
     *     nf::Context m_context;
     *     int m_data{};
     * };
     * @endcode
     *
     * @par Example
     *
     * @code
     * Context ctx;
     * auto f = ctx->bind([](int i) { return i * 5; });
     * int result{0};
     * f(10).then(ctx, [&](std::optional<int> val) { result = *val; });
     * //result is 50
     * @endcode
     */
    template <typename tFunc, typename = std::enable_if_t<!detail::IsNonCallableBindable<tFunc>>>
    [[nodiscard]] auto bind(tFunc &&f) const noexcept
    {
        return m_state->bind(std::forward<tFunc>(f));
    }

    /**
     * @brief Takes an arbitrary callable and makes it a deferred one.
     *
     * This function binds the provided callable @p f to this context and returns a wrapper
     * callable with the same @e look-and-feel as @p f. Once the wrapper is invoked the
     * callable @p f is posted to the underlying executor of this context.
     *
     * If this instance of @ref Context is @ref reset() "reset" or destroyed, the wrapper
     * becomes a no-op function and all associated resources (e.g. captured objects in a
     * lambda) are freed.
     *
     * NOTE: When the created wrapper is destroyed it's not guaranteed that underlying captured
     * resources are released immediately. The delay can be caused by pending invocation in the
     * event loop queue
     *
     * @par Example
     *
     * @code
     * class MyClass
     * {
     * public:
     *     void initializeData()
     *     {
     *         auto handleEvent = m_context.bindDeferred([this](int data)
     *         {
     *             // This callback will be invoked only if "this" is a valid pointer,
     *             // thus it is safe to access m_data. The actual exectuion will be done
     *             // on the context's executor.
     *             m_data = data;
     *         });
     *         ExternalComponent::getDataAsync(handleEvent);
     *     }
     *
     * private:
     *     // Context as a member of a class ensures that all callbacks bound to this context
     *     // will be invalidated when an object of MyClass is destroyed.
     *     nf::Context m_context;
     *     int m_data{};
     * };
     * @endcode
     *
     * @since 4.11
     */
    template <typename tFunc>
    [[nodiscard]] auto bindDeferred(tFunc &&f) const noexcept
    {
        return m_state->bindDeferred(std::forward<tFunc>(f));
    }

    /**
     * @brief Bind a signal subscription.
     *
     * This function transfers the ownership of a signal subscription from @p sub to this
     * context, i.e. it binds the subscription lifetime to this context.
     *
     * If this instance of @ref Context is @ref reset() "reset" or destroyed, all bound
     * subscriptions become terminated.
     *
     * @par Example
     *
     * @code
     * nf::Context context;
     * context.bind(signal.subscribe([] { ... }));
     * @endcode
     *
     * @sa @ref nf_core_Signals
     * @since 4.18
     */
    void bind(Subscription &&sub) const noexcept;

    /**
     * @brief Bind a @ref RaiiToken to this @ref Context.
     *
     * If this instance of @ref Context is @ref reset() "reset" or destroyed the action of the
     * bound token is executed.
     */
    void bind(RaiiToken &&token) const noexcept;

    /**
     * @brief Bind a context aware item.
     *
     * This function binds the provided @ref ContextItem to this context. The context takes the
     * ownership of the provided <b>shared pointer</b>, not the context item itself. It is up to
     * a caller to decide if they want to keep any other references or @c std::move() the last
     * one into this function.
     *
     * @warning An item cannot be bound to more than one context. An attempt to bind an already
     * bound item will terminate the application.
     *
     * If this instance of @ref Context is @ref reset() "reset" or destroyed, the stored shared
     * pointer is released. This can cause the context aware item to be immediately deleted, if
     * this happens to be the last pointer to it.
     *
     * The item can unbind itself by using @nfref{ContextItem::decontextualize()}.
     *
     * @code
     * nf::Context context;
     * auto item = makeSomeContextAwareItem();
     * context.bind(std::move(item)); // Move!
     *
     * // Item is deleted here as the context holds the last reference
     * context.reset();
     * @endcode
     *
     * Note, however, that the item will @b not be deleted if there are other references to it.
     *
     * @code
     * auto item = makeSomeContextAwareItem();
     * context.bind(item); // Copy!
     * context.reset();    // Item instance is NOT deleted
     * @endcode
     */
    void bind(std::shared_ptr<ContextItem> item) const noexcept;

    /**
     * @brief Check if the context is @e empty.
     *
     * This function returns true if there are no entities associated with this context, i.e.
     * no bound callables, signal subscriptions etc.
     */
    bool isEmpty() const noexcept;

    /**
     * @brief Reset the context.
     *
     * This function resets this context. This means, e.g. all bound callables or signal
     * subscriptions are invalidated. This has the same effect as destroying the context
     * and creating a new one in its place.
     *
     * Note, this function is not thread-safe. No other function must be called while the
     * context is being reset.
     */
    void reset() noexcept;

    /**
     * @brief Check if the context is bound to the caller's thread.
     *
     * This function returns true if this context is bound to the executor of the @ref
     * Executor::thisThread() "current thread".
     *
     * @since 4.12
     */
    bool isThisThread() const noexcept;

private:
    friend class detail::ContextState;
    std::shared_ptr<detail::ContextState> m_state;
};

NF_END_NAMESPACE
