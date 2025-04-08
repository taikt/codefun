/*
 * BMW Neo Framework
 *
 * Copyright (C) 2020-2021 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include <nf/Enums.h>
#include <nf/Printable.h>
#include <nf/testing/Test.h>
#include <nf/testing/TestIoStream.h>

#include <optional>
#include <set>
#include <vector>

NF_ENUM(PrintableEnum, One, Two)

template <typename T>
std::string printableToString(const T &t) noexcept
{
    std::stringstream ss;
    ss << nf::printable(t);
    return ss.str();
}

template <std::size_t N>
struct SimplePodClass
{
    std::array<char, N> data;
};


struct NonPodClass : private virtual SimplePodClass<5>
{
public:
    int publicA{10};
    std::int16_t publicB{23};
};

TEST(PrintableTest, intVector)
{
    // Empty vector:
    EXPECT_EQ("(0)[]", printableToString(std::vector<int>()));

    // Two elements:
    EXPECT_EQ("(2)[3, 55]", printableToString(std::vector<int>({3, 55})));
}

TEST(PrintableTest, stringVector)
{
    // std::string:
    EXPECT_EQ("(3)[\"Hello\", \"my\", \"world!\"]",
              printableToString(std::vector<std::string>({"Hello", "my", "world!"})));

    // const char*:
    EXPECT_EQ("(3)[\"Hello\", \"my\", \"world!\"]",
              printableToString(std::vector<const char *>({"Hello", "my", "world!"})));
}

TEST(PrintableTest, intSet)
{
    // Empty set:
    EXPECT_EQ("(0)[]", printableToString(std::set<int>()));

    // Two elements:
    EXPECT_EQ("(2)[3, 55]", printableToString(std::set<int>({3, 55})));
}

TEST(PrintableTest, stringSet)
{
    // std::string:
    EXPECT_EQ("(2)[\"Hello\", \"world!\"]",
              printableToString(std::set<std::string>({"Hello", "world!", "Hello"})));

    // const char*:
    // NOTE: AnyOf is used because the order is not specified
    EXPECT_THAT(printableToString(std::set<const char *>({"Hello", "my", "world!", "my"})),
                testing::AnyOf("(3)[\"Hello\", \"my\", \"world!\"]",
                               "(3)[\"Hello\", \"world!\", \"my\"]"));
}

TEST(PrintableTest, stringIntPair)
{
    EXPECT_EQ("{\"ONE\", 1}", printableToString(std::pair<std::string, int>("ONE", 1)));
}

TEST(PrintableTest, intStringDoubleTuple)
{
    std::tuple<int, std::string, double> t{42, "hello", 2.8};
    EXPECT_EQ("{42, \"hello\", 2.8}", printableToString(t));
}

TEST(PrintableTest, int2intMap)
{
    EXPECT_EQ("(2)[{1, 2}, {3, 4}]", printableToString(std::map<int, int>{{1, 2}, {3, 4}, {1, 5}}));
}

TEST(PrintableTest, int2intMultimap)
{
    EXPECT_EQ("(3)[{1, 2}, {1, 5}, {3, 4}]",
              printableToString(std::multimap<int, int>{{1, 2}, {3, 4}, {1, 5}}));
}

TEST(PrintableTest, optional)
{
    EXPECT_EQ("<nullopt>", printableToString(std::optional<int>()));
    EXPECT_EQ("(1)[42]", printableToString(std::optional<std::vector<int>>{{42}}));
    EXPECT_EQ("Two", printableToString(std::optional<PrintableEnum>(PrintableEnum::Two)));
    EXPECT_EQ("30", printableToString(std::optional<std::uint8_t>(30)));
}

TEST(PrintableTest, uint8)
{
    EXPECT_EQ("0", printableToString(std::uint8_t{0}));
    EXPECT_EQ("42", printableToString(std::uint8_t{42}));
}

TEST(PrintableTest, byte)
{
    EXPECT_EQ("00", printableToString(std::byte{0x00}));
    EXPECT_EQ("AB", printableToString(std::byte{0xab}));
}

TEST(PrintableTest, enum)
{
    enum testEnum
    {
        Zero,
        One,
    };
    enum class testSignedEnum : int
    {
        Zero,
        One,
    };
    enum class testUnsignedEnum : unsigned
    {
        Zero,
        One,
    };

    EXPECT_EQ("0", printableToString(Zero));
    EXPECT_EQ("1", printableToString(One));
    EXPECT_EQ("0", printableToString(testSignedEnum::Zero));
    EXPECT_EQ("1", printableToString(testSignedEnum::One));
    EXPECT_EQ("0", printableToString(testUnsignedEnum::Zero));
    EXPECT_EQ("1", printableToString(testUnsignedEnum::One));
}

TEST(PrintableTest, referenceWrapper)
{
    int i = 50;
    std::reference_wrapper<int> w{i};
    EXPECT_EQ("50", printableToString(w));
}

namespace {
// clang-format off
class One {};
class Two {};
class Three : boost::noncopyable {};

[[maybe_unused]] const char *toString(One) { return "toString(One)"; }
[[maybe_unused]] const char *toString(Two) { return "toString(Two)"; }

std::ostream &operator<<(std::ostream &os, Two) { return os << "stream(Two)"; }
std::ostream &operator<<(std::ostream &os, const Three &) { return os << "stream(Three)"; }
// clang-format on
} // anonymous namespace

TEST(PrintableTest, toStringAndOStream)
{
    // Only toString() is available:
    EXPECT_EQ("toString(One)", printableToString(One()));

    // Both are available, ostream must be preferred:
    EXPECT_EQ("stream(Two)", printableToString(Two()));

    // Only ostream operator is available:
    EXPECT_EQ("stream(Three)", printableToString(Three()));
}

TEST(PrintableTest, strerror)
{
    EXPECT_EQ("Numerical argument out of domain", nf::strerror(EDOM));
}

TEST(PrintableTest, hexRepresentation_podClass)
{
    const auto podStr = printableToString(SimplePodClass<3>{'1', '2', 'D'});
    EXPECT_FALSE(podStr.empty());
    EXPECT_EQ(podStr, "SimplePodClass<3ul>{31 32 44}");
}

TEST(PrintableTest, hexRepresentation_podClassWithSymbols_A_and_F)
{
    const auto podStr = printableToString(
        SimplePodClass<5>{char{0}, char{0x1f}, char{0x1D}, char{0x1A}, char{0x0B}});
    EXPECT_FALSE(podStr.empty());
    EXPECT_EQ(podStr, "SimplePodClass<5ul>{00 1F 1D 1A 0B}");
}

TEST(PrintableTest, hexRepresentation_complicatedClass)
{
    // just check that we can get a string representation for a complicated object with the padding,
    // mixing of visibility modifiers
    const auto podStr = printableToString(NonPodClass{});
    EXPECT_FALSE(podStr.empty());
}

TEST(PrintableTest, hexRepresentation_bigObjectDotsExpected)
{
    SimplePodClass<100> obj{'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A',  // 10xA
                            'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B', 'B',  // 10xB
                            'C', 'C', 'C', 'C', 'C', 'C', 'C', 'C', 'C', 'C',  // 10xC
                            'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D', 'D',  // 10xD
                            'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E', 'E',  // 10xE
                            'F', 'F', 'F', 'F', 'F', 'F', 'F', 'F', 'F', 'F',  // 10xF
                            'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G', 'G',  // 10xG
                            'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H', 'H'}; // 10xH
    const auto convertedString = printableToString(obj);

    EXPECT_FALSE(convertedString.empty());

    constexpr auto *expectedString =
        "SimplePodClass<100ul>{"
        "41 41 41 41 41 41 41 41 41 41 " // 10xA
        "42 42 42 42 42 42 42 42 42 42 " // 10xB
        "43 43 43 43 43 43 43 43 43 43 " // 10xC
        "44 44 44 44 44 44 44 44 44 44 " // 10xD
        "45 45 45 45 45 45 45 45 45 45 " // 10xE
        "46 46 46 46 46 46 46 46 46 46 " // 10xF
        "47 47 47 47 ...}";              // 4xG

    EXPECT_EQ(convertedString, expectedString);
}
