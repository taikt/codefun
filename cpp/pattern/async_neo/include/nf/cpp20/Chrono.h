/*
 * BMW Neo Framework
 *
 * Copyright (C) 2020 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Jan Ehrenberger <jan.ehrenberger@bmw.de>
 */

#pragma once

#include <nf/Global.h>

#include <chrono>
#include <type_traits>

NF_BEGIN_NAMESPACE

/// C++20 features.
namespace cpp20 {

/**
 * @ingroup nf_Core
 * @brief Convert @p timePoint to time point of different clock @c tDestClock.
 * @param timePoint Time point using @c tSourceClock.
 * @return Representation of time point using @c tDestClock.
 *
 * This is a very simple implementation of @cppdoc{clock_cast,chrono/clock_cast} from C++20.
 *
 * @note @c tSourceClock and @c tDestClock must meet requirements for
 *       @cppdoc{Clock,named_req/Clock}, mainly providing the @c now function.
 */
template <typename tDestClock, typename tSourceClock, typename tDuration,
          typename = typename std::enable_if_t<!std::is_same<tDestClock, tSourceClock>::value>>
auto clock_cast(const std::chrono::time_point<tSourceClock, tDuration> &timePoint)
{
    return tDestClock::now() + (timePoint - tSourceClock::now());
}

template <typename tSourceClock, typename tDuration>
auto clock_cast(const std::chrono::time_point<tSourceClock, tDuration> &timePoint)
{
    return timePoint;
}

} // namespace cpp20

NF_END_NAMESPACE
