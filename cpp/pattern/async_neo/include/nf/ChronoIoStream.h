/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Thomas Leichtle <thomas.leichtle@bmw.de>
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#if __cplusplus > 201703L
#error C++20 defines streaming operators for std::chrono. Do not use this header.
#endif

#pragma once

#include <nf/Global.h>

#include <chrono>
#include <iomanip>
#include <iostream>

NF_BEGIN_NAMESPACE

namespace detail {

// clang-format off
template <typename tRatio> inline constexpr char ChronoSuffix[]    = "x";
template <> inline constexpr char ChronoSuffix<std::nano>[]        = "ns";
template <> inline constexpr char ChronoSuffix<std::micro>[]       = "µs";
template <> inline constexpr char ChronoSuffix<std::milli>[]       = "ms";
template <> inline constexpr char ChronoSuffix<std::ratio<1>>[]    = "s";
template <> inline constexpr char ChronoSuffix<std::ratio<60>>[]   = "min";
template <> inline constexpr char ChronoSuffix<std::ratio<3600>>[] = "h";
// clang-format on

} // namespace detail

NF_END_NAMESPACE

namespace std { // NOLINT
namespace chrono {

/**
 * @ingroup nf_core_Logging
 * @brief @cppdoc{C++20-like,chrono/duration/operator_ltlt} streaming operator for
 *        @e std::chrono::duration types.
 */
template <typename tRep, typename tRatio>
std::ostream &operator<<(std::ostream &os, const duration<tRep, tRatio> &duration) noexcept
{
    os << duration.count();

    constexpr const char *const suffix = nf::detail::ChronoSuffix<tRatio>;
    if (suffix[0] != 'x') {
        return os << suffix;
    }

    os << "*" << tRatio::num;
    if (tRatio::den == 1) {
        return os << "s";
    }
    return os << "/" << tRatio::den << "s";
}

/**
 * @ingroup nf_core_Logging
 * @brief Streaming operator for @cppdoc{std::chrono::time_point,chrono/time_point}.
 */
template <typename tClock, typename tDuration>
std::ostream &operator<<(std::ostream &os, const time_point<tClock, tDuration> &point) noexcept
{
    const auto t = tClock::to_time_t(point);
    struct tm buf; // NOLINT
    return os << std::put_time(::localtime_r(&t, &buf), "%F %T %Z");
}

} // namespace chrono
} // namespace std
