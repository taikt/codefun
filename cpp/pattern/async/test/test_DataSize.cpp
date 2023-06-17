/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include <nf/DataSize.h>
#include <nf/testing/Test.h>
#include <nf/testing/TestIoStream.h>

using namespace nf;
using namespace nf::testing;
using namespace nf::unit_literals;

template <typename T, class tRatio>
class TestSize : public nf::detail::AbstractUnit<TestSize, T, tRatio>
{
    using nf::detail::AbstractUnit<TestSize, T, tRatio>::AbstractUnit;
};

using TestBytes = TestSize<std::int64_t, std::ratio<1>>;
using TestKiB = TestSize<std::int64_t, std::ratio<1024>>;
using TestMiB = TestSize<std::int64_t, std::ratio<1048576>>;
using TestGiB = TestSize<std::int64_t, std::ratio<1073741824>>;

template <typename tUnit1, typename tUnit2>
void expectIsRelated(bool expectation) noexcept
{
    const bool isRelated = nf::detail::IsRelatedUnit<tUnit1, tUnit2>::value;
    EXPECT_EQ(expectation, isRelated);
}

TEST(DataSizeTest, isRelatedUnit)
{
    // clang-format off
    expectIsRelated<Bytes,     Kibibytes>(true);
    expectIsRelated<Bytes,     Mebibytes>(true);
    expectIsRelated<Bytes,     Kibibytes>(true);
    expectIsRelated<Kibibytes, Gibibytes>(true);
    expectIsRelated<TestBytes, TestGiB>(true);
    expectIsRelated<TestMiB,   TestGiB>(true);

    expectIsRelated<Bytes,     TestBytes>(false);
    expectIsRelated<Kibibytes, TestKiB>(false);
    expectIsRelated<Mebibytes, TestGiB>(false);
    expectIsRelated<TestMiB,   Gibibytes>(false);
    // clang-format on
}

TEST(DataSizeTest, customLiterals)
{
    auto bytes = 124_B;
    EXPECT_EQ(124, bytes.value());

    auto kib = 125_KiB;
    EXPECT_EQ(125, kib.value());

    auto mib = 126_MiB;
    EXPECT_EQ(126, mib.value());

    auto gib = 127_GiB;
    EXPECT_EQ(127, gib.value());
}

TEST(DataSizeTest, streaming)
{
    EXPECT_EQ("10 B", streamToString(10_B));
    EXPECT_EQ("20 KiB", streamToString(20_KiB));
    EXPECT_EQ("30 MiB", streamToString(30_MiB));
    EXPECT_EQ("40 GiB", streamToString(40_GiB));
}

TEST(DataSizeTest, equality)
{
    // Same unit:
    EXPECT_LT(1_KiB, 2_KiB);
    EXPECT_LE(1_KiB, 2_KiB);
    EXPECT_LE(2_KiB, 2_KiB);
    EXPECT_EQ(2_KiB, 2_KiB);
    EXPECT_GE(2_KiB, 2_KiB);
    EXPECT_GT(3_KiB, 2_KiB);
    EXPECT_GE(3_KiB, 2_KiB);

    // Different units:
    EXPECT_LT(2047_B, 2_KiB);
    EXPECT_GT(2_KiB, 2047_B);

    EXPECT_EQ(2048_B, 2_KiB);
    EXPECT_EQ(2_KiB, 2048_B);

    EXPECT_GT(2049_B, 2_KiB);
    EXPECT_LT(2_KiB, 2049_B);
}

TEST(DataSizeTest, additiveOperators)
{
    Mebibytes mib{100};
    EXPECT_EQ(101_MiB, mib + 1_MiB);
    EXPECT_EQ(99_MiB, mib - 1_MiB);

    EXPECT_EQ(1025_B, 1_B + 1_KiB);
    EXPECT_EQ(1025_B, 1_KiB + 1_B);

    mib += 1_GiB;
    EXPECT_EQ(1124_MiB, mib);

    mib -= 120_MiB;
    EXPECT_EQ(1004_MiB, mib);
}

TEST(DataSizeTest, mulDivByFactor)
{
    EXPECT_EQ(150_GiB, 50_GiB * 3);
    EXPECT_EQ(150_GiB, 3 * 50_GiB);
    EXPECT_EQ(100_GiB, 200_GiB / 2);
}

TEST(DataSizeTest, conversions)
{
    // Implicit (safe) conversions:
    Bytes bytes = 349_MiB;
    EXPECT_EQ(365953024_B, bytes);

    Kibibytes kib = 349_MiB;
    EXPECT_EQ(357376_KiB, kib);

    // Explicit (unsafe) conversions:
    kib = datasize_cast<Kibibytes>(500_B);
    EXPECT_EQ(0_KiB, kib);

    kib = datasize_cast<Kibibytes>(520_B);
    EXPECT_EQ(0_KiB, kib);

    // Expressions:
    auto res = 1_KiB + 1_B + 1_MiB + 1_GiB;
    EXPECT_EQ(1074791425, res.value());
}

TEST(DataSizeTest, ratio)
{
    auto ratio = 1_KiB / 2_B;
    EXPECT_DOUBLE_EQ(512.0, ratio); // 1KiB is 512 times larger than 2 bytes

    ratio = 1_KiB / 2_KiB;
    EXPECT_DOUBLE_EQ(0.5, ratio);
}

TEST(DataSizeTest, zeroTest)
{
    Bytes bytes;
    EXPECT_EQ(0, bytes.value());
}
