/*
 * BMW Neo Framework
 *
 * Copyright (C) 2020 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Jan Ehrenberger <jan.ehrenberger@bmw.de>
 */

#include <nf/cpp20/Chrono.h>
#include <nf/testing/Test.h>

using namespace std::chrono;

namespace {
constexpr std::chrono::seconds MinClockCastPrecision{1};
} // anonymous namespace

TEST(ChronoTest, castSteadyToSystemClock)
{
    auto steadyPoint = steady_clock::now();
    auto systemPoint = nf::cpp20::clock_cast<system_clock>(steadyPoint);
    EXPECT_TRUE(abs(steadyPoint - nf::cpp20::clock_cast<steady_clock>(systemPoint))
                <= MinClockCastPrecision);
}

TEST(ChronoTest, castSystemToHighResolutionClock)
{
    auto systemPoint = system_clock::now();
    auto highResPoint = nf::cpp20::clock_cast<high_resolution_clock>(systemPoint);
    EXPECT_TRUE(abs(systemPoint - nf::cpp20::clock_cast<system_clock>(highResPoint))
                <= MinClockCastPrecision);
}

TEST(ChronoTest, castHighResolutionToSteadyClock)
{
    auto highResPoint = high_resolution_clock::now();
    auto steadyPoint = nf::cpp20::clock_cast<steady_clock>(highResPoint);
    EXPECT_TRUE(abs(highResPoint - nf::cpp20::clock_cast<high_resolution_clock>(steadyPoint))
                <= MinClockCastPrecision);
}

TEST(ChronoTest, castSameClock)
{
    auto steadyPoint = steady_clock::now();
    EXPECT_EQ(steadyPoint, nf::cpp20::clock_cast<steady_clock>(steadyPoint));
}
