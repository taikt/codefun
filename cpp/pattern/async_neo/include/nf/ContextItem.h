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

#include <nf/Function.h>

#include <boost/core/noncopyable.hpp>

#include <list>
#include <memory>

NF_BEGIN_NAMESPACE

class Context;
class ContextItem;

namespace detail {

class ContextState;

/* This allows doing context->bind() on a forward declared Context. */
void bind(const Context &context, std::shared_ptr<ContextItem> item) noexcept;

} // namespace detail

/**
 * @ingroup nf_core_ExecutionFlow
 * @brief The context aware item.
 *
 * This is a base class for items which can be bound to @nfref{Context}.
 *
 * @par Usage
 *
 * Inherit this class to implement anything, which lifetime needs to be bound and guarded by a
 * context.
 *
 * @note An instance of a context aware item @b must be a shared pointer.
 *
 * It can be bound to a context using @ref Context::bind(std::shared_ptr<ContextItem>) const
 * "Context::bind()". It can also unbind itself with @ref decontextualize().
 *
 * @code
 * class MyJob : public nf::ContextItem
 * {
 * };
 *
 * context.bind(std::make_shared<MyJob>());
 * @endcode
 *
 * @note Due to the nature of shared pointers, it is <b>not possible</b> to bind an item to a
 * context at construction time.
 *
 * @par Lifetime
 *
 * Lifetime management is based on shared pointers. An instance of this class is alive while there
 * is at least one shared pointer referencing this instance. A context can hold one of the
 * references, but it might <b>be not the only</b> reference.
 *
 * For example:
 *
 * @code
 * auto job = std::make_shared<MyJob>();
 * context.bind(std::move(job));
 * context.reset(); // MyJob instance is deleted
 * @endcode
 *
 * but
 *
 * @code
 * auto job = std::make_shared<MyJob>();
 * context.bind(job); // Copy!
 * context.reset();   // MyJob instance is NOT deleted
 * job.reset();       // Now, it is deleted
 * @endcode
 */
class ContextItem : boost::noncopyable, public std::enable_shared_from_this<ContextItem>
{
public:
    using Task = Function<void()>;
    using List = std::list<std::shared_ptr<ContextItem>>;
    using Iterator = List::const_iterator;

protected:
    ContextItem() noexcept;
    virtual ~ContextItem();

protected:
    /**
     * @brief Decontextualize this item.
     *
     * This function unbinds this item from the context. If the item is not bound to any context,
     * it will be a no-op.
     *
     * @note This instance of @ref ContextItem might get destroyed inside this call. Subclasses
     * must @b not access their local state (i.e. @c this) after invoking this method.
     *
     * @par Example
     *
     * @code
     * class MyJob : public nf::ContextItem
     * {
     *     // This function is invoked when some job performed by the context aware item (not
     *     // shown in this example) is done
     *     void onJobDone()
     *     {
     *         nf::info("Job is done, destroying the object");
     *         decontextualize();
     *     }
     * };
     *
     * auto job = std::make_shared<MyJob>();
     * context.bind(std::move(job));
     * @endcode
     */
    void decontextualize() noexcept;

    /**
     * @brief Post a task to the bound context.
     *
     * This function posts a task to the bound context. The posted task will be invoked if and
     * only if @b both the context and the item still exist when the execution is due.
     *
     * @par Example
     *
     * @code
     * class MyJob : public nf::ContextItem
     * {
     * public:
     *     void start()
     *     {
     *         post([this] {
     *             // This will not be invoked if either the item does not have the context, the
     *             // context gets destroyed, or the item gets destroyed
     *             doSomething();
     *         });
     *     }
     * };
     * @endcode
     */
    void post(Task &&task) noexcept;

private:
    friend class detail::ContextState;
    void contextualize(std::weak_ptr<detail::ContextState> &&state, Iterator it) noexcept;

private:
    Iterator m_it;
    std::weak_ptr<detail::ContextState> m_state;
};

NF_END_NAMESPACE
