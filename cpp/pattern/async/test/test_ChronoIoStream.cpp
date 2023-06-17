/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include <nf/ChronoIoStream.h>
#include <nf/testing/Test.h>
#include <nf/testing/TestIoStream.h>

#include <chrono>
#include <cstdlib>

using namespace nf::testing;

TEST(ChronoIoStreamTest, genericChronoTypes)
{
    using namespace std::chrono_literals;

    EXPECT_EQ("1h", streamToString(1h));
    EXPECT_EQ("2min", streamToString(2min));
    EXPECT_EQ("3s", streamToString(3s));
    EXPECT_EQ("4ms", streamToString(4ms));
    EXPECT_EQ("5µs", streamToString(5us));
    EXPECT_EQ("6ns", streamToString(6ns));
}

TEST(ChronoIoStreamTest, customChronoTypes)
{
    std::chrono::duration<int, std::milli> ms{1};
    EXPECT_EQ("1ms", streamToString(ms));

    std::chrono::duration<int, std::ratio<2>> twos{2};
    EXPECT_EQ("2*2s", streamToString(twos));

    std::chrono::duration<int, std::ratio<2, 3>> twoThirds{3};
    EXPECT_EQ("3*2/3s", streamToString(twoThirds));
}

TEST(ChronoIoStreamTest, timePoint)
{
    /* The streaming operator includes a current timezone, so we need to explicitly
     * set the current timezone so this test works worldwide. */
    char utc[] = "TZ=UTC";
    putenv(utc); // NOLINT(concurrency-mt-unsafe)

    auto time = std::chrono::system_clock::from_time_t(1562160806);
    EXPECT_EQ("2019-07-03 13:33:26 UTC", streamToString(time));
}
