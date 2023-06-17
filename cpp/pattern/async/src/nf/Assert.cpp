/*
 * BMW Neo Framework
 *
 * Copyright (C) 2018-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include "nf/Assert.h"

void nf::detail::nf_assert(const std::ostringstream &errMsg, const char *fileName,
                           const char *funcName, int line) noexcept
{
    detail::log(NF_LOG_GET_CONTEXT(nf_core), nf::LogLevel::Fatal, "Assertion failed. {}",
                fmt::make_format_args(errMsg.str()),
                nf::source_location::current(fileName, funcName, line));

    std::abort();
}

void nf::detail::nf_expect(const std::ostringstream &errMsg, const char *fileName,
                           const char *funcName, int line) noexcept
{
    detail::log(NF_LOG_GET_CONTEXT(nf_core), nf::LogLevel::Fatal, "Expectation failed. {}",
                fmt::make_format_args(errMsg.str()),
                nf::source_location::current(fileName, funcName, line));
}

void nf::detail::logAssert(std::string_view msgPrefix, FormatString formatStr,
                           fmt::format_args formatArgs)
{
    std::string newFormatStr{msgPrefix};
    newFormatStr += formatStr.format.data();
    detail::log(NF_LOG_GET_CONTEXT(nf_core), nf::LogLevel::Fatal, newFormatStr, formatArgs,
                formatStr.location);
}

void nf::detail::handleAssert(FormatString formatStr, fmt::format_args formatArgs)
{
    detail::logAssert("Assertion Failed. ", formatStr, formatArgs);

    std::abort();
}

[[nodiscard]] bool nf::detail::handleExpect(FormatString formatStr, fmt::format_args formatArgs)
{
    detail::logAssert("Expectation Failed. ", formatStr, formatArgs);

    return false;
}

[[noreturn]] void nf::detail::handleFail(FormatString formatStr, fmt::format_args formatArgs)
{
    detail::logAssert("Assertion Failed. ", formatStr, formatArgs);

    std::abort();
}
