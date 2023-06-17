/*
 * BMW Neo Framework
 *
 * Copyright (C) 2020 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include <nf/OptionalIoStream.h>
#include <nf/testing/Test.h>
#include <nf/testing/TestIoStream.h>

#include <vector>

using namespace nf::testing;

NF_ENUM(OptEnum, One, Two, Three)

namespace optns {
struct OptClass
{
    std::optional<int> value;
};
std::ostream &operator<<(std::ostream &os, const OptClass &value)
{
    return os << "OptClass: " << value.value;
}
} // namespace optns

TEST(OptionalIoStreamTest, integer)
{
    EXPECT_EQ("<nullopt>", streamToString(std::optional<int>{}));
    EXPECT_EQ("99", streamToString(std::optional<int>{99}));
}

TEST(OptionalIoStreamTest, intVector)
{
    EXPECT_EQ("<nullopt>", streamToString(std::optional<std::vector<int>>{}));
    EXPECT_EQ("(4)[1, 2, 3, 4]", streamToString(std::optional<std::vector<int>>{{1, 2, 3, 4}}));
}

TEST(OptionalIoStreamTest, nfEnums)
{
    EXPECT_EQ("<nullopt>", streamToString(std::optional<OptEnum>{}));
    EXPECT_EQ("Two", streamToString(std::optional<OptEnum>{OptEnum::Two}));
}

TEST(OptionalIoStreamTest, string)
{
    EXPECT_EQ("<nullopt>", streamToString(std::optional<std::string>{}));
    EXPECT_EQ("\"hello\"", streamToString(std::optional<std::string>{"hello"}));
}

TEST(OptionalIoStreamTest, customClass)
{
    EXPECT_EQ("<nullopt>", streamToString(std::optional<optns::OptClass>{}));
    optns::OptClass optClass;
    EXPECT_EQ("OptClass: <nullopt>", streamToString(std::optional<optns::OptClass>{optClass}));
    optClass.value = 77;
    EXPECT_EQ("OptClass: 77", streamToString(std::optional<optns::OptClass>{optClass}));
}
