/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019-2021 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#pragma once

#include <nf/Global.h>

#include <boost/noncopyable.hpp>

#include <memory>

NF_BEGIN_NAMESPACE

namespace detail {

class InvocableNtBase : boost::noncopyable
{
public:
    virtual ~InvocableNtBase() = default;
};

template <bool IsNoexcept, typename tReturn, typename... tArgs>
class InvocableBase : public InvocableNtBase
{
public:
    virtual tReturn invoke(tArgs... args) noexcept(IsNoexcept) = 0;
};

template <bool IsNoexcept, typename tFunc, typename tReturn, typename... tArgs>
class Invocable final : public InvocableBase<IsNoexcept, tReturn, tArgs...>
{
public:
    template <typename xFunc,
              typename = std::enable_if_t<
                  !std::is_base_of_v<InvocableNtBase, std::remove_reference_t<xFunc>>>>
    explicit Invocable(xFunc &&f) noexcept
        : m_f(std::forward<xFunc>(f))
    {
    }

    tReturn invoke(tArgs... args) noexcept(IsNoexcept) override
    {
        if constexpr (std::is_void_v<tReturn>) {
            m_f(std::forward<tArgs>(args)...);
        } else {
            return m_f(std::forward<tArgs>(args)...);
        }
    }

private:
    std::decay_t<tFunc> m_f;
};

class FunctionNtBase
{
public:
    FunctionNtBase() noexcept = default;
    FunctionNtBase(FunctionNtBase &&) = default;
    FunctionNtBase &operator=(FunctionNtBase &&) = default;

protected:
    [[noreturn]] static void terminateOnEmptyTarget() noexcept;
    [[noreturn]] static void throwOnEmptyTarget();
};

template <bool IsNoexcept, typename tReturn, typename... tArgs>
class FunctionBase : public FunctionNtBase
{
public:
    FunctionBase() noexcept = default;

    template <typename tFunc,
              typename = std::enable_if_t<
                  !std::is_base_of_v<FunctionNtBase, std::remove_reference_t<tFunc>>>>
    FunctionBase(tFunc &&f) noexcept // NOLINT(google-explicit-constructor)
        : m_f(std::make_unique<Invocable<IsNoexcept, tFunc, tReturn, tArgs...>>(
            std::forward<tFunc>(f)))
    {
    }

    template <typename... xArgs>
    tReturn operator()(xArgs &&...args) const noexcept(IsNoexcept)
    {
        if (!m_f) {
            if constexpr (IsNoexcept) {
                terminateOnEmptyTarget();
            } else {
                throwOnEmptyTarget();
            }
        }

        return m_f->invoke(std::forward<xArgs>(args)...);
    }

    explicit operator bool() const noexcept
    {
        return static_cast<bool>(m_f);
    }

private:
    friend class AnyFunction;
    using InvocableType = detail::InvocableBase<IsNoexcept, tReturn, tArgs...>;
    std::unique_ptr<InvocableType> m_f;
};

class AnyFunction;

} // namespace detail

/**
 * @ingroup nf_core_Functional
 * @brief Type trait to check if the given type is an instance of @ref nf::Function.
 */
template <typename T>
inline constexpr bool IsFunction =
    std::is_base_of<detail::FunctionNtBase, std::remove_reference_t<T>>::value;

/**
 * @ingroup nf_core_Functional
 * @brief The function wrapper.
 *
 * @code
 * #include <nf/Function.h>
 * @endcode
 *
 * This is a general-purpose function wrapper, similar to @cppdoc{std::function,utility/
 * functional/function}, which can store and invoke any @e callable @e object with the given
 * signature, i.e. different kinds of functions and functors, e.g. lambdas. Such callable
 * object is called a @e target of @c nf::Function. If there is no target, @c nf::Function
 * is called @e empty.
 *
 * In contrast to @c std::function, @c nf::Function is non-copyable and thus can store
 * non-copyable targets.
 *
 * If an empty @c nf::Function is invoked:
 * - For @e noexcept targets @c std::terminate() is called.
 * - Else, @cppdoc{std::bad_function_call,utility/functional/bad_function_call} is thrown.
 *
 * @b Examples
 *
 * @code nf::Function<int()> f; @endcode
 * Defines @e f as a function which returns @c int and accepts no arguments.
 *
 * @code nf::Function<void(int,double)> f; @endcode
 * Defines @e f as a function which has not return value and accepts @c int and @c double
 * as arguments.
 *
 * @code nf::Function<void() noexcept> f; @endcode
 * Defines @e f as a function which has no return value, no arguments and throws no exceptions.
 *
 * @code
 * auto p = std::make_unique<int>(42);
 * nf::Function<void()> f = [p = std::move(p)] { ... };
 * @endcode
 * Assigns a non-copyable lambda to @c nf::Function.
 */
template <typename tSignature>
class Function;

template <typename tReturn, typename... tArgs>
class Function<tReturn(tArgs...)> final : public detail::FunctionBase<false, tReturn, tArgs...>
{
    using detail::FunctionBase<false, tReturn, tArgs...>::FunctionBase;
};

template <typename tReturn, typename... tArgs>
class Function<tReturn(tArgs...) noexcept> final
    : public detail::FunctionBase<true, tReturn, tArgs...>
{
    using detail::FunctionBase<true, tReturn, tArgs...>::FunctionBase;
};

NF_END_NAMESPACE
