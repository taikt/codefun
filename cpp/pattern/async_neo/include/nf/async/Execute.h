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

#include <nf/Context.h>
#include <nf/Promise.h>
#include <nf/Thread.h>
#include <nf/detail/AsyncSharedState.h>

#include <boost/hana/functional/partial.hpp>

NF_BEGIN_NAMESPACE

namespace detail {

template <typename F, typename... tArgs>
auto prepareExecution(F &func, tArgs &...args) noexcept
{
    static_assert(std::is_invocable_v<F, tArgs...>, "Cannot call type F with Args...");

    using R = std::invoke_result_t<F, tArgs...>;

    auto promise = Promise<R>{};
    auto future = promise.future();
    auto invocator = InvocationHelper<F, R, tArgs...>{std::move(func), std::move(promise)};

    return std::make_tuple( //
        [f = boost::hana::partial(std::move(invocator), std::move(args)...)]() mutable {
            std::move(f)();
        },
        std::move(future));
}

} // namespace detail

namespace async {

/**
 * @addtogroup nf_core_Async
 * @{
 */

/**
 * @brief Execute functions asynchronously via the event loop.
 *
 * @param context Context of the function.
 * @param func    Function to execute.
 * @param args    Function arguments.
 *
 * @return @nfref{Future} which can be used to attach a continuation.
 */
template <typename F, typename... tArgs>
auto execute(const Context &context, F func, tArgs... args) noexcept
{
    auto [callable, future] = detail::prepareExecution(func, args...);
    context.bindDeferred(std::move(callable))();
    return std::move(future);
}

/**
 * @brief Execute a function asynchronously in a provided worker thread.
 *
 * @param thread Thread to execute a function in.
 * @param func   Function to execute.
 * @param args   Function arguments.
 *
 * @return @nfref{Future} which can be used to attach a continuation.
 * @since 4.16
 */
template <typename F, typename... tArgs>
auto execute(Thread &thread, F func, tArgs... args) noexcept
{
    auto [callable, future] = detail::prepareExecution(func, args...);
    thread.post(std::move(callable));
    return std::move(future);
}

/// @}

} // namespace async

NF_END_NAMESPACE
