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

#include <tuple>
#include <type_traits>

NF_BEGIN_NAMESPACE

namespace detail {

template <typename tReturn, typename... tArgs>
struct FunctionTraitsImpl
{
    using Signature = tReturn(tArgs...);
    using ReturnType = tReturn;
    using ArgTypes = std::tuple<typename std::remove_cv_t<std::remove_reference_t<tArgs>>...>;
    static constexpr std::size_t NrArgs = sizeof...(tArgs);

    template <std::size_t idx>
    using ArgType = std::tuple_element_t<idx, ArgTypes>;
};

template <typename tReturn>
struct FunctionTraitsImpl<tReturn>
{
    using Signature = tReturn();
    using ReturnType = tReturn;
    using ArgTypes = std::tuple<>;
    static constexpr std::size_t NrArgs = 0;
};

} // namespace detail

/**
 * @ingroup nf_core_Functional
 * @brief Function traits.
 *
 * @code
 * #include <nf/FunctionTraits.h>
 * @endcode
 *
 * This template provides a type information about an arbitrary type of callable (a function,
 * a lambda, a functor etc.): its return type, its number and types of arguments.
 *
 * @b Usage
 *
 * @code
 * auto lambda = [](const int&, double} {};
 * using FT = FunctionTraits<decltype(lambda)>;
 * @endcode
 * defines a type trait @c FT which has these properties:
 *
 * - @code FT::Signature @endcode
 *   Signature of a callable, <code>void(const int&, double)</code> in this case.
 *
 * - @code FT::ReturnType @endcode
 *   Return type of a callable, @c void in this case.
 *
 * - @code FT::ArgTypes @endcode
 *   A tuple with callable argument types (dereferenced and without cv-qualifiers),
 *   @c std::tuple<int, double> in this case.
 *
 * - @code FT::NrArgs @endcode
 *   A number of arguments, 2 in this case.
 *
 * - @code FT::ArgType<idx> @endcode
 *   A type (dereferenced and without cv-qualifiers) of the given argument, e.g.
 *   @c FT::ArgType<0> is a type of the first argument, @c int in this case.
 *
 * @b Limitations
 *
 * This template cannot be used for:
 * - Functors with an overloaded @c operator().
 * - Overloaded functions.
 * - Generic lambdas.
 */
template <typename tFunc>
struct FunctionTraits;

/// @cond
template <typename tFunc>
struct FunctionTraits : FunctionTraits<decltype(&std::remove_reference_t<tFunc>::operator())>
{
};

template <typename tReturnType, typename tClassType, typename... tArgs>
struct FunctionTraits<tReturnType (tClassType::*)(tArgs...) const>
    : detail::FunctionTraitsImpl<tReturnType, tArgs...>
{
};

template <typename tReturnType, typename tClassType, typename... tArgs>
struct FunctionTraits<tReturnType (tClassType::*)(tArgs...)>
    : detail::FunctionTraitsImpl<tReturnType, tArgs...>
{
};

template <typename tReturnType, typename tClassType, typename... tArgs>
struct FunctionTraits<tReturnType (tClassType::*&)(tArgs...)>
    : detail::FunctionTraitsImpl<tReturnType, tArgs...>
{
};

template <typename tReturnType, typename... tArgs>
struct FunctionTraits<tReturnType (*)(tArgs...)> : detail::FunctionTraitsImpl<tReturnType, tArgs...>
{
};

template <typename tReturnType, typename... tArgs>
struct FunctionTraits<tReturnType (&)(tArgs...)> : detail::FunctionTraitsImpl<tReturnType, tArgs...>
{
};

template <typename tReturnType, typename... tArgs>
struct FunctionTraits<tReturnType(tArgs...)> : detail::FunctionTraitsImpl<tReturnType, tArgs...>
{
};
/// @endcond

NF_END_NAMESPACE
