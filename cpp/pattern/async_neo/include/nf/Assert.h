/*
 * BMW Neo Framework
 *
 * Copyright (C) 2018-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 *     Thomas Leichtle <thomas.leichtle@bmw.de>
 */

#pragma once

#include <nf/Logging.h>

#include <boost/preprocessor/facilities/overload.hpp>

#include <sstream>

/**
 * @addtogroup nf_core_Assert
 * @{
 */

#define impl_NF_ASSERT_DEPRECATED                                                                  \
    _Pragma(                                                                                       \
        "message (\"\
NF_ASSERT, NF_EXPECT and NF_FAIL macros are deprecated. Please consider using new functions \
like nf::assertThat() which all also support libfmt formatting style.\")")

#define impl_NF_ASSERT(func, cond, errmsg, retstatement)                                           \
    impl_NF_ASSERT_DEPRECATED;                                                                     \
    do {                                                                                           \
        if (__builtin_expect(!(cond), 0)) {                                                        \
            std::ostringstream _nf_os;                                                             \
            _nf_os << errmsg;                                                                      \
            ::nf::detail::func(_nf_os, __FILE__, __FUNCTION__, __LINE__);                          \
            retstatement                                                                           \
        }                                                                                          \
    } while (!42)

#define impl_NF_EXPECT_1(cond) _Pragma("GCC error \"NF_EXPECT must accept two or three arguments\"")
#define impl_NF_EXPECT_2(cond, errmsg) impl_NF_ASSERT(nf_expect, cond, errmsg, return;)
#define impl_NF_EXPECT_3(cond, errmsg, retval)                                                     \
    impl_NF_ASSERT(nf_expect, cond, errmsg, return retval;)

/**
 * @hideinitializer
 * @brief Check that the given assertion is true.
 *
 * @par Syntax
 * @code
 * NF_ASSERT(cond, errmsg)
 * @endcode
 *
 * <dl class="params"><dt>Parameters</dt><dd><table class="params">
 *     <tr><td class="paramname">cond</td><td>Condition to be checked.</td></tr>
 *     <tr><td class="paramname">errmsg</td><td>Error message.</td></tr>
 * </table></dd></dl>
 *
 * This macro checks if the given condition @p cond is true. If not, it logs an error message
 * @p errmsg and terminates the application with @c std::abort().
 *
 * @sa @ref NF_FAIL()
 * @sa @ref NF_EXPECT()
 */
#define NF_ASSERT(cond, errmsg) impl_NF_ASSERT(nf_assert, cond, errmsg, /*noreturn*/)

/**
 * @hideinitializer
 * @brief Trigger an unconditional failure.
 *
 * @par Syntax
 * @code
 * NF_FAIL(errmsg)
 * @endcode
 *
 * This macro expands to @ref NF_ASSERT(false, errmsg).
 */
#define NF_FAIL(errmsg) NF_ASSERT(false, errmsg)

/**
 * @hideinitializer
 * @brief Check that the given expectation is true.
 *
 * @par Syntax
 * @code
 * NF_EXPECT(cond, errmsg)
 * NF_EXPECT(cond, errmsg, retval)
 * @endcode
 *
 * <dl class="params"><dt>Parameters</dt><dd><table class="params">
 *     <tr><td class="paramname">cond</td><td>Condition to be checked.</td></tr>
 *     <tr><td class="paramname">errmsg</td><td>Error message.</td></tr>
 *     <tr><td class="paramname">retval</td><td><em>(optional)</em> Return value.</td></tr>
 * </table></dd></dl>
 *
 * This is a non-terminating version of @ref NF_ASSERT(). This macro checks if the given
 * condition @p cond is true. If not, it logs an error message @p errmsg and returns
 * @p retval from the current function.
 */
#define NF_EXPECT(...) BOOST_PP_OVERLOAD(impl_NF_EXPECT_, __VA_ARGS__)(__VA_ARGS__)

/// @} // nf_core_Assert

NF_BEGIN_NAMESPACE

namespace detail {

[[noreturn]] void nf_assert(const std::ostringstream &errMsg, const char *fileName,
                            const char *funcName, int line) noexcept;
void nf_expect(const std::ostringstream &errMsg, const char *fileName, const char *funcName,
               int line) noexcept;

void logAssert(std::string_view msgPrefix, FormatString formatStr, fmt::format_args formatArgs);
void handleAssert(FormatString formatStr, fmt::format_args formatArgs);
bool handleExpect(FormatString formatStr, fmt::format_args formatArgs);
[[noreturn]] void handleFail(FormatString formatStr, fmt::format_args formatArgs);

} // namespace detail

template <typename tCond, typename... tArgs>
void assertThat(tCond cond, FormatString formatStr, tArgs &&...args)
{
    if (!cond) {
        detail::handleAssert(formatStr, fmt::make_format_args(args...));
    }
}

template <typename tCond, typename... tArgs>
[[nodiscard]] bool expectThat(tCond cond, FormatString formatStr, tArgs &&...args)
{
    if (!cond) {
        return detail::handleExpect(formatStr, fmt::make_format_args(args...));
    }
    return true;
}

template <typename... tArgs>
[[noreturn]] void fail(FormatString formatStr, tArgs &&...args)
{
    detail::handleFail(formatStr, fmt::make_format_args(args...));
}

NF_END_NAMESPACE
