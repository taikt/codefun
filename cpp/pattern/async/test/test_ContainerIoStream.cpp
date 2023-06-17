/*
 * BMW Neo Framework
 *
 * Copyright (C) 2018 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include <nf/ContainerIoStream.h>
#include <nf/testing/Test.h>
#include <nf/testing/TestIoStream.h>

#include <vector>

#include <fmt/ostream.h>

using nf::testing::streamToString;

TEST(ContainerIoStreamTest, intVector)
{
    // Empty vector:
    EXPECT_EQ("(0)[]", streamToString(std::vector<int>()));

    // Two elements:
    EXPECT_EQ("(2)[3, 55]", streamToString(std::vector<int>({3, 55})));
}

TEST(ContainerIoStreamTest, intVector_libfmt)
{
    // Empty vector:
    EXPECT_EQ("(0)[]", fmt::format("{}", std::vector<int>()));

    // Two elements:
    EXPECT_EQ("(2)[3, 55]", fmt::format("{}", std::vector<int>({3, 55})));
}

TEST(ContainerIoStreamTest, byteVector)
{
    // Two elements:
    EXPECT_EQ("(2)[3, 55]", streamToString(std::vector<uint8_t>({3, 55})));

    // iomanip applied correctly
    std::stringstream ss;
    ss << std::setbase(16) << std::showbase << std::vector<uint8_t>(11, 0x42);
    auto val = ss.str();

    EXPECT_THAT(val, testing::StartsWith("(11)[0x42, 0x42,"));
}

TEST(ContainerIoStreamTest, stringVector)
{
    // std::string:
    EXPECT_EQ("(3)[\"Hello\", \"my\", \"world!\"]",
              streamToString(std::vector<std::string>({"Hello", "my", "world!"})));

    // const char*:
    EXPECT_EQ("(3)[\"Hello\", \"my\", \"world!\"]",
              streamToString(std::vector<const char *>({"Hello", "my", "world!"})));
}

TEST(ContainerIoStream, intSet)
{
    // Empty set:
    EXPECT_EQ("(0)[]", streamToString(std::set<int>()));

    // Two elements:
    EXPECT_EQ("(2)[3, 55]", streamToString(std::set<int>({3, 55})));
}

TEST(ContainerIoStream, stringSet)
{
    // std::string:
    EXPECT_EQ("(2)[\"Hello\", \"world!\"]",
              streamToString(std::set<std::string>({"Hello", "world!", "Hello"})));

    // const char*:
    // NOTE: AnyOf is used because the order is not specified
    EXPECT_THAT(streamToString(std::set<const char *>({"Hello", "my", "world!", "my"})),
                testing::AnyOf("(3)[\"Hello\", \"my\", \"world!\"]",
                               "(3)[\"Hello\", \"world!\", \"my\"]"));
}

TEST(ContainerIoStreamTest, stringIntPair)
{
    EXPECT_EQ("{\"ONE\", 1}", streamToString(std::pair<std::string, int>("ONE", 1)));
}

TEST(ContainerIoStreamTest, tuple)
{
    std::tuple<int, std::string, double, uint8_t> t{42, "hello", 2.8, 31};
    EXPECT_EQ("{42, \"hello\", 2.8, 31}", streamToString(t));
}

TEST(ContainerIoStreamTest, int2intMap)
{
    EXPECT_EQ("(2)[{1, 2}, {3, 4}]", streamToString(std::map<int, int>{{1, 2}, {3, 4}, {1, 5}}));
}

TEST(ContainerIoStreamTest, int2intMultimap)
{
    EXPECT_EQ("(3)[{1, 2}, {1, 5}, {3, 4}]",
              streamToString(std::multimap<int, int>{{1, 2}, {3, 4}, {1, 5}}));
}

// clang-format off
class A {};
class B {};
class C {};

const char *toString(A) { return "toString(A)"; }
const char *toString(B) { return "toString(B)"; }

std::ostream &operator<<(std::ostream &os, B) { return os << "stream(B)"; }
std::ostream &operator<<(std::ostream &os, C) { return os << "stream(C)"; }
// clang-format on

TEST(ContainerIoStreamTest, toStringAndOStream)
{
    // Only toString() is available:
    EXPECT_EQ("(1)[toString(A)]", streamToString(std::vector<A>({A()})));

    // Both are available, ostream must be preferred:
    EXPECT_EQ("(1)[stream(B)]", streamToString(std::vector<B>({B()})));

    // Only ostream operator is available:
    EXPECT_EQ("(1)[stream(C)]", streamToString(std::vector<C>({C()})));
}
