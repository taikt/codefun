/*
 * BMW Neo Framework
 *
 * Copyright (C) 2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Manuel Nickschas <manuel.nickschas@bmw.de>
 */

#pragma once

#include <nf/Global.h>

#include <optional>
#include <type_traits>

NF_BEGIN_NAMESPACE

namespace detail {

template <typename T>
struct IsOptional : public std::false_type
{
};

template <template <typename> class T, typename O>
struct IsOptional<T<O>> : public std::is_same<T<O>, std::optional<O>>
{
};

} // namespace detail

/**
 * @ingroup nf_core_Types
 * @brief Detect if type T is std::optional<O> for any value type O.
 *
 * @since 5.3
 */
template <typename T>
inline constexpr bool IsOptional = detail::IsOptional<T>::value;

NF_END_NAMESPACE
