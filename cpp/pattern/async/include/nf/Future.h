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

#include <nf/Promise.h>
#include <nf/Result.h>
#include <nf/detail/AsyncSharedState.h>

#include <boost/hana/type.hpp>

#include <utility>

NF_BEGIN_NAMESPACE

namespace detail {

template <typename tValue, typename tStorage = tValue>
class FutureBase
{
    static_assert(!std::is_reference_v<tValue>, "Cannot construct Future for a reference");

public:
    using ValueType = tValue;
    using PromiseType = Promise<tValue>;

public:
    FutureBase(const FutureBase &) noexcept = default;
    FutureBase &operator=(const FutureBase &) noexcept = default;
    FutureBase(FutureBase &&) noexcept = default;
    FutureBase &operator=(FutureBase &&) noexcept = default;

    template <typename F>
    auto then(const Context &context, F func) const noexcept
    {
        if constexpr (std::is_void_v<tValue>) {
            static_assert(std::is_invocable_v<F>, "Continuation must be invocable with 'void'");
        } else {
            static_assert(std::is_invocable_v<F, tValue>,
                          "Continuation must be invocable with 'TValue'");
        }

        using NextValue = typename std::conditional_t<std::is_void_v<tValue>, std::invoke_result<F>,
                                                      std::invoke_result<F, tValue>>::type;

        auto continuation =
            std::make_shared<Continuation<F, tStorage, NextValue>>(context, std::move(func));
        auto future = continuation->future();

        m_state->setContinuation(context, std::move(continuation));
        return future;
    }

protected:
    friend Promise<tValue>;
    using SharedState = AsyncSharedState<tStorage>;

protected:
    FutureBase() = default;

    explicit FutureBase(std::shared_ptr<SharedState> state) noexcept
        : m_state{std::move(state)}
    {
    }

protected:
    std::shared_ptr<SharedState> m_state = std::make_shared<SharedState>();
};

} // namespace detail

/**
 * @ingroup nf_core_Async
 * @brief Future value. Allows attaching continuation to asynchronous promise.
 *
 * Continuation function will be executed via the executor from the bound context. It will be
 * executed even if future is destroyed after attaching continuation. However, if the @c context
 * goes away, continuation will not be executed.
 *
 * @tparam tValue Type of value produced by the promise
 */
template <typename tValue>
class Future final : public detail::FutureBase<tValue>
{
public:
#ifdef DOXYGEN
    /**
     * @brief Value type of this @c Future.
     */
    using ValueType = tValue;

    /**
     * @brief Type of @nfref{Promise} associated with this Future.
     */
    using PromiseType = Promise<tValue>;
#endif

public:
    /**
     * Construct ready future of the type.
     *
     * Future created by this ctor has a value, thus attached continuation will be sent
     * to the event loop for execution immediately.
     *
     * @param value Value for the continuation
     */
    // NOLINTNEXTLINE(google-explicit-constructor)
    Future(tValue value) noexcept
        : detail::FutureBase<tValue>()
    {
        this->m_state->setValue(std::move(value));
    }

    /**
     * Construct future from a promise.
     *
     * @param[in] p Promise to construct from
     */
    explicit Future(const Promise<tValue> &p) noexcept
        : detail::FutureBase<tValue>(p.m_state)
    {
    }

#ifdef DOXYGEN
    /**
     * @brief Attach any callable as continuation.
     *
     * Continuation must be callable with one argument of the type, contained by this Future.
     * @warning Must be called exactly once. Calling it second time results in program termination.
     *
     * @param context      Context for continuation.
     * @param continuation Anything, which can be called with the value type as argument
     * @return Future of next operation
     */
    auto then(const nf::Context &context, F continuation) const noexcept;

    /**
     * @brief Attach any callable as continuation and call it only if previous operation succeeded.
     *
     * When there is a @nfref{Future} which has a @nfref{Result} as its value, this
     * function can be used to handle only success cases. This simplifies longer async chains by
     * removing the need for checking the result at each step. The continuation must also return a
     * @nfref{Result} type. The returned @nfref{Result} type can have a different @c Value type but
     * must have the same @c Error type.
     *
     * @note This method is defined only for @nfref{Future} of @nfref{Result}.
     *
     * @warning Must be called exactly once. Calling it a second time results in program
     * termination.
     *
     * @param context      Context for continuation.
     * @param continuation Anything which can be called with the value type as argument.
     * @return Future of the next operation
     *
     * @since 4.19
     */
    auto whenOk(const nf::Context &context, F &&continuation) const noexcept;

    /**
     * Unwrap a nested Future to get a "flat" Future of the underlying value.
     *
     * @note Applicable only to nested futures e.g. @c Future<Future<int>>.
     *
     * @param context Context for the continuation.
     */
    Future<tValue> unwrap(const nf::Context &context) const noexcept;
#endif
};

namespace detail {

template <typename T>
struct IsFuture : std::false_type
{
};

template <template <typename> typename T, typename tValue>
struct IsFuture<T<tValue>> : std::is_same<T<tValue>, Future<tValue>>
{
};

template <typename T>
struct FutureValueTypeBase : std::false_type
{
};

template <template <typename> typename T, typename tValue>
struct FutureValueTypeBase<T<tValue>>
{
    using value_type = tValue;
};

template <typename T>
using FutureValueType = typename FutureValueTypeBase<T>::value_type;

} // namespace detail

/**
 * @brief Type trait to check if the given type is an instance of @nfref{Future}.
 */
template <typename T>
inline constexpr bool IsFuture = detail::IsFuture<T>::value;

template <>
class Future<void> : public detail::FutureBase<void, std::monostate>
{
public:
    Future() noexcept
    {
        m_state->setValue({});
    }

    explicit Future(const Promise<void> &p) noexcept
        : detail::FutureBase<void, std::monostate>(p.m_state)
    {
    }
};

template <typename tValue>
class Future<Future<tValue>> : public detail::FutureBase<tValue, Future<tValue>>
{
public:
    // NOLINTNEXTLINE(google-explicit-constructor)
    Future(Future<tValue> inner) noexcept
    {
        this->m_state->setValue(std::move(inner));
    }

    explicit Future(const Promise<Future<tValue>> &p) noexcept
        : detail::FutureBase<tValue, Future<tValue>>(p.m_state)
    {
    }

    Future<tValue> unwrap(const nf::Context &context) const noexcept
    {
        if constexpr (std::is_void_v<tValue>) {
            return this->then(context, []() {});
        } else {
            return this->then(context, [](tValue &&value) { return std::move(value); });
        }
    }

    template <typename F, typename T = tValue,
              typename std::enable_if_t<nf::IsResult<T>, bool> = true>
    auto whenOk(const Context &context, F &&continuation) const noexcept
    {
        return this->unwrap(context).whenOk(context, std::forward<F>(continuation));
    }
};

template <typename tValue, typename tError>
class Future<nf::Result<tValue, tError>> : public detail::FutureBase<nf::Result<tValue, tError>>
{
public:
    using typename detail::FutureBase<nf::Result<tValue, tError>>::ValueType;

public:
    // NOLINTNEXTLINE(google-explicit-constructor)
    Future(ValueType value) noexcept
        : detail::FutureBase<ValueType>()
    {
        this->m_state->setValue(std::move(value));
    }

    explicit Future(Promise<ValueType> p) noexcept
        : detail::FutureBase<ValueType>(p.m_state)
    {
    }

    template <typename F>
    auto whenOk(const Context &context, F &&continuation) const noexcept
    {
        if constexpr (std::is_void_v<tValue>) {
            static_assert(std::is_invocable_v<F>, "Continuation must be invocable with 'void'");
        } else {
            static_assert(
                std::is_invocable_v<F, tValue>,
                "Continuation must be invocable with the 'tValue' of the 'Result<tValue, tError>'");
        }

        using tReturnValue =
            typename std::conditional_t<std::is_void_v<tValue>, std::invoke_result<F>,
                                        std::invoke_result<F, tValue>>::type;

        constexpr auto validateTypes = [](auto t) {
            using ReturnValueType = typename decltype(t)::type;
            static_assert(
                nf::IsResult<ReturnValueType>,
                "Continuation must return a 'Result<...>' or a 'Future<Result<...>>' type");
            static_assert(
                std::is_same_v<typename ReturnValueType::ErrorType, typename ValueType::ErrorType>,
                "'ErrorType' of the continuation return must match error type of this Future");
        };

        if constexpr (IsFuture<tReturnValue>) {
            validateTypes(boost::hana::type_c<typename tReturnValue::ValueType>);
        } else {
            validateTypes(boost::hana::type_c<tReturnValue>);
        }

        return this->then(context,
                          [continuation =
                               std::forward<F>(continuation)](ValueType result) -> tReturnValue {
                              if (result) {
                                  if constexpr (std::is_void_v<tValue>) {
                                      return continuation();
                                  } else {
                                      return continuation(std::move(result).value());
                                  }
                              }

                              if constexpr (std::is_void_v<tError>) {
                                  return {nf::result::Err{}};
                              } else {
                                  return {nf::result::Err{result.errorValue()}};
                              }
                          });
    }
};

template <typename tValue, typename tError = void>
using FutureResult = Future<Result<tValue, tError>>;

NF_END_NAMESPACE
