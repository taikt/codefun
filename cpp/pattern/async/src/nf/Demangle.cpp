/*
 * BMW Neo Framework
 *
 * Copyright (C) 2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Florian Franzmann <Florian.Franzmann@ul.com>
 *     Manuel Nickschas <manuel.nickschas@bmw.de>
 */

#include "nf/Demangle.h"

#include <boost/core/demangle.hpp>

#include <regex>

NF_BEGIN_NAMESPACE

namespace {

std::string removeNamespace(std::string_view demangled) noexcept
{
    // Remove all namespaces
    static std::regex s_regex(R"((\w+::)+(\w+))");
    return std::regex_replace(demangled.data(), s_regex, "$2");
}

} // namespace

std::string demangleWithoutNamespace(std::string_view mangled) noexcept
{
    return removeNamespace(demangle(mangled));
}

std::string demangle(std::string_view mangled) noexcept
{
    return boost::core::demangle(mangled.data());
}

NF_END_NAMESPACE
