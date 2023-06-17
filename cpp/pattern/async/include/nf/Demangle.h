/*
 * BMW Neo Framework
 *
 * Copyright (C) 2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Florian Franzmann <Florian.Franzmann@ul.com>
 */

#pragma once

#include <nf/Global.h>

#include <string>
#include <string_view>
#include <typeinfo>

NF_BEGIN_NAMESPACE

/**
 * @ingroup nf_Core
 */

/**
 * @brief Demangle a name.
 *
 * This function converts the (mangled) name used internally by the compiler for
 * a class or function to human-readable form.
 *
 * @param mangled The mangled type name.
 */
[[nodiscard]] std::string demangle(std::string_view mangled) noexcept;

/**
 * @brief Demangle a type name removing the namespace.
 *
 * @param mangled The mangled type name.
 */
[[nodiscard]] std::string demangleWithoutNamespace(std::string_view mangled) noexcept;

/**
 * @brief Demangle a type name.
 *
 * This function converts the name of a type to human-readable form.
 *
 * @tparam T The type to convert.
 */
template <typename T>
[[nodiscard]] std::string demangle() noexcept
{
    return demangle(typeid(T).name());
}

/**
 * @brief Demangle a type name.
 *
 * This function converts the name of a type to human-readable form, removing
 * namespaces.
 *
 * @tparam T The type to convert.
 */
template <typename T>
[[nodiscard]] std::string demangleWithoutNamespace() noexcept
{
    return demangleWithoutNamespace(typeid(T).name());
}

NF_END_NAMESPACE
