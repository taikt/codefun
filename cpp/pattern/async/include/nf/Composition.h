/*
 * BMW Neo Framework
 *
 * Copyright (C) 2021-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Alexey Larikov <alexey.larikov@bmw.de>
 *     Manuel Nickschas <manuel.nickschas@bme.de>
 */

#pragma once

#include <nf/Assert.h>
#include <nf/Context.h>
#include <nf/ContextItem.h>
#include <nf/Executor.h>
#include <nf/Future.h>
#include <nf/Promise.h>
#include <nf/Result.h>
#include <nf/TypeTraits.h>

#include <chrono>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

NF_BEGIN_NAMESPACE

namespace async {

struct Timeout
{
};

} // namespace async

namespace detail {

namespace when_any {

template <typename std::size_t I, typename tVariant, typename tPromise, typename tResult>
void fulfillVariantPromise(const std::shared_ptr<tPromise> &promisePtr, tResult &&value) noexcept
{
    if (auto &promise = *promisePtr; promise.has_value()) {
        promise->fulfill(tVariant{std::in_place_index<I>, std::forward<tResult>(value)});
        promise = std::nullopt;
    }
}

template <typename std::size_t I, typename tVariant, typename tPromise, typename tResult>
void attachOne(const nf::Context &context, const std::shared_ptr<tPromise> &promisePtr,
               const Future<tResult> &future) noexcept
{
    if constexpr (std::is_void_v<tResult>) {
        future.then(context, [promisePtr]() {
            fulfillVariantPromise<I, tVariant>(promisePtr, std::monostate{});
        });
    } else {
        future.then(context, [promisePtr](auto result) {
            fulfillVariantPromise<I, tVariant>(promisePtr, std::move(result));
        });
    }
}

template <typename... tResult, std::size_t... Is>
auto attach(const nf::Context &context, std::tuple<Future<tResult> &&...> futures,
            std::index_sequence<Is...> /*unused*/) noexcept
{
    using TVariant =
        std::variant<std::conditional_t<std::is_void_v<tResult>, std::monostate, tResult>...>;
    using TPromise = Promise<TVariant>;

    auto promisePtr = std::make_shared<std::optional<TPromise>>(TPromise{});

    (attachOne<Is, TVariant>(context, promisePtr, std::get<Is>(futures)), ...);

    return promisePtr->value().future();
}

} // namespace when_any

namespace when_all {

template <typename std::size_t I, typename tFinalTuple, typename tPromise, typename tWaitingTuple,
          typename tResult>
void fulfilledOne(std::shared_ptr<tPromise> promisePtr, std::shared_ptr<tWaitingTuple> waitingTuple,
                  tResult &&value) noexcept
{
    assertThat(!std::get<I>(*waitingTuple).has_value(), "This promise has been processed already");

    std::get<I>(*waitingTuple) = {std::forward<tResult>(value)};

    constexpr auto kReady = [](const auto &...args) { return (... && args.has_value()); };

    constexpr auto kToFinal = [](auto &&...args) -> tFinalTuple {
        return tFinalTuple(std::move(args).value()...);
    };

    if (std::apply(kReady, *waitingTuple)) {
        promisePtr->fulfill(std::apply(kToFinal, std::move(*waitingTuple)));
    }
}

template <typename std::size_t I, typename tFinalTuple, typename tPromise, typename tWaitingTuple,
          typename tResult>
void attachOne(const nf::Context &context, const std::shared_ptr<tPromise> &promisePtr,
               const std::shared_ptr<tWaitingTuple> &waitingTuple,
               const Future<tResult> &future) noexcept
{
    if constexpr (std::is_void_v<tResult>) {
        future.then(context, [promisePtr, waitingTuple]() mutable {
            fulfilledOne<I, tFinalTuple>(std::move(promisePtr), std::move(waitingTuple),
                                         std::monostate{});
        });
    } else {
        future.then(context, [promisePtr, waitingTuple]([[maybe_unused]] auto result) mutable {
            fulfilledOne<I, tFinalTuple>(std::move(promisePtr), std::move(waitingTuple),
                                         std::move(result));
        });
    }
}

template <typename... tResult, std::size_t... Is>
auto attach(const nf::Context &context, std::tuple<Future<tResult> &&...> futures,
            std::index_sequence<Is...> /*unused*/) noexcept
{
    using tWaitingTuple =
        std::tuple<std::conditional_t<std::is_void_v<tResult>, std::optional<std::monostate>,
                                      std::optional<tResult>>...>;
    using tFinalTuple =
        std::tuple<std::conditional_t<std::is_void_v<tResult>, std::monostate, tResult>...>;
    using tPromise = Promise<tFinalTuple>;

    auto waitingTuple = std::make_shared<tWaitingTuple>(tWaitingTuple{});
    auto promisePtr = std::make_shared<tPromise>(tPromise{});

    (attachOne<Is, tFinalTuple>(context, promisePtr, waitingTuple, std::get<Is>(futures)), ...);

    return promisePtr->future();
}

} // namespace when_all

namespace until_done {

template <typename tFunc, typename tResult>
class UntilDone final : public ContextItem
{
public:
    explicit UntilDone(tFunc func) noexcept
        : m_func{std::move(func)}
    {
    }

    auto future() noexcept
    {
        post([this] { iterate(); });
        return m_promise.future().then(m_context, [this](auto &&result) {
            decontextualize(); // self-destruct once done
            return std::forward<decltype(result)>(result);
        });
    }

private:
    void iterate()
    {
        if constexpr (!nf::IsFuture<std::result_of_t<tFunc()>>) {
            checkResult(m_func());
        } else {
            m_func().then(m_context, [&](auto &&result) {
                checkResult(std::forward<decltype(result)>(result));
            });
        }
    }

    template <typename R>
    void checkResult(R &&result)
    {
        if (result) {
            m_promise.fulfill(std::forward<R>(result).value());
        } else {
            post([this] { iterate(); });
        }
    }

private:
    Context m_context;
    tFunc m_func;
    Promise<tResult> m_promise;
};

template <typename tFunc, typename tResult>
auto start(const Context &context, tFunc func)
{
    auto untilDone = std::make_shared<UntilDone<tFunc, tResult>>(std::move(func));
    context.bind(untilDone);
    return untilDone->future();
}

} // namespace until_done

} // namespace detail

/**
 * @ingroup nf_core_Async
 * @brief Returns a future which completes when one of the passed-in futures completes
 *
 * The future created by this function will contain a `std::variant` of all result types of the
 * passed-in futures. When one of the passed-in futures completes, its result is set as
 * a value of a variant. The client then can inspect the variant to get the result. Once one
 * future is ready all other results will be ignored.
 *
 * @note In case any of the futures have a `void` result type, it will be replaced with
 * `std::monostate` in the resulting `std::variant`.
 *
 * @param context Scope for the continuations
 * @param futures Futures to wait for one of them to complete
 *
 * @return Future with variant of all passed-in futures
 */
template <typename... tResult>
auto whenAny(const nf::Context &context, Future<tResult> &&...futures) noexcept
{
    using Ixs = std::index_sequence_for<tResult...>;
    return detail::when_any::attach(context, std::forward_as_tuple(std::move(futures)...), Ixs{});
}

/**
 * @ingroup nf_core_Async
 * @brief Returns future which completes when all of the passed-in future have completed.
 *
 * The future created by this function will contain a `std::tuple` of all result types of the
 * passed-in futures. When all of the passed-in futures are completed, its result is set to
 * a tuple containing all values. The client then can inspect the tuple to get the results.
 *
 * @note In case any of the futures have a `void` result type, it will be replaced with
 * `std::monostate` in the resulting `std::tuple`.
 *
 * @param context Scope for the continuations
 * @param futures Futures to wait for one of them to complete
 *
 * @return Future with tuple of all passed-in futures
 */
template <typename... tResult>
auto whenAll(const nf::Context &context, Future<tResult> &&...futures) noexcept
{
    using Ixs = std::index_sequence_for<tResult...>;
    return detail::when_all::attach(context, std::forward_as_tuple(std::move(futures)...), Ixs{});
}

/**
 * @ingroup nf_core_Async
 * @brief Returns a future, which is fulfilled after a certain time interval
 *
 * This is useful as a building block for timed operations, for example, one can implement logic
 * "when all, not earlier than" or "when any, no later than".
 *
 * @code
 *     // concurrent operations with a timeout, first result wins
 *     auto f1 = whenAny(ctx, makeTimeout(ctx, 1s), downloadFromMirror1(), downloadFromMirror2());
 *
 *     // wait for multiple values and report both no earlier than in one second
 *     auto f2 = whenAll(ctx, makeTimeout(ctx, 1s), getSignal1(), getSignal2());
 *     f2.then([](auto v) {
 *          publishValue(v);
 *     })
 *
 * @endcode
 *
 * @param context Scope for the timeout future
 * @param timeout Timeout to wait for in milliseconds
 *
 * @return Future with of @c Timeout struct
 * @since 4.17
 **/
inline auto makeTimeout(const nf::Context &context, std::chrono::milliseconds timeout)
{
    auto p = Promise<async::Timeout>{};
    context.post(timeout, [p]() { p.fulfill({}); });
    return p.future();
}

/**
 * @ingroup nf_core_Async
 * @brief Returns a future which completes either when the given future completes or the timeout is
 * reached
 *
 * The future created by this function will contain a `std::variant` of the result type of the
 * passed-in future. When either the passed future or the internal timer future completes, its
 * result is set as a value of a variant. The client then can inspect the variant to get the result.
 * Once one future is ready all other results will be ignored.
 *
 * @note In case the future has a `void` result type, it will be replaced with `std::monostate` in
 * the resulting `std::variant`.
 *
 * @param context Scope for the continuations
 * @param timeout Timeout to wait for in milliseconds
 * @param future Future to wait for to complete
 *
 * @return Future with variant of either the result type or a @c Timeout struct.
 */
template <typename tResult>
auto expiresIn(const nf::Context &context, std::chrono::milliseconds timeout,
               Future<tResult> &&future) noexcept
{
    return whenAny(context, std::forward<Future<tResult>>(future), makeTimeout(context, timeout));
}

/**
 * @ingroup nf_core_Async
 * @brief Returns a future which completes once a function has provided a result after potentially
 *        several iterations.
 *
 * The function must return either a @c std::optional<T> or a @c Future<std::optional<T>> for
 * an arbitrary value type @c T. It will be invoked repeatedly via the event loop, until
 * it returns something other than @c std::nullopt. This allows, for example, for incremental
 * processing of some task.
 *
 * Execution will be aborted if the provided context is reset or destroyed.
 *
 * @param context Context guarding the lifetime of execution
 * @param func    Function to invoke, potentially repeatedly until it returns a valid result
 *
 * @return Future to wait for to complete
 *
 * @since 5.3
 */
template <typename tFunc, typename tReturn = std::result_of_t<tFunc()>>
auto untilDone(const Context &context, tFunc func)
{
    if constexpr (nf::IsFuture<tReturn>) {
        static_assert(IsOptional<typename tReturn::ValueType>,
                      "Future type has to be std::optional");
        using ResultType = typename tReturn::ValueType::value_type;
        return detail::until_done::start<tFunc, ResultType>(context, std::move(func));
    } else {
        static_assert(IsOptional<tReturn>, "Return type has to be std::optional");
        using ResultType = typename tReturn::value_type;
        return detail::until_done::start<tFunc, ResultType>(context, std::move(func));
    }
}

NF_END_NAMESPACE
