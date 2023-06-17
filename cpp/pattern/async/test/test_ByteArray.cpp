/*
 * BMW Neo Framework
 *
 * Copyright (C) 2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Kutyavin Nikolay <nikolay.kutyavin@bmw.de>
 *     Holger Kaelberer <holger.kaelberer@bmw.de>
 */

#include <nf/ByteArray.h>
#include <nf/testing/Test.h>

#include <algorithm>
#include <cstring>
#include <iterator>
#include <vector>

using namespace nf;
using namespace nf::testing;
using namespace bytearray_literals;

namespace {

ByteArray asciiByteArray()
{
    static const auto Ba = []() {
        ByteArray ba;
        ba.resize(1 << 8);
        uint8_t c{};
        std::transform(ba.begin(), ba.end(), ba.begin(),
                       [&c](const auto &) { return static_cast<std::byte>(c++); });
        return ba;
    }();
    return Ba;
}

const auto AsciiBase64 = std::string{
    "AAECAwQFBgcICQoLDA0ODxAREhMUFRYXGBkaGxwdHh8gISIjJCUmJygpKissLS4vMDEyMzQ1Njc4OTo7PD0+"
    "P0BBQkNERUZHSElKS0xNTk9QUVJTVFVWV1hZWltcXV5fYGFiY2RlZmdoaWprbG1ub3BxcnN0dXZ3eHl6e3x9fn+"
    "AgYKDhIWGh4iJiouMjY6PkJGSk5SVlpeYmZqbnJ2en6ChoqOkpaanqKmqq6ytrq+wsbKztLW2t7i5uru8vb6/"
    "wMHCw8TFxsfIycrLzM3Oz9DR0tPU1dbX2Nna29zd3t/g4eLj5OXm5+jp6uvs7e7v8PHy8/T19vf4+fr7/P3+/w=="};

} // namespace

class ByteArrayTest : public Test
{
};

TEST_F(ByteArrayTest, CopyConstructor)
{
    const std::string storedContent = "1234";
    const std::vector<std::byte> expectedData = {std::byte{0x31}, std::byte{0x32}, std::byte{0x33},
                                                 std::byte{0x34}};
    ByteArray arrOriginal = ByteArray::fromString(storedContent);
    ByteArray arrCopy = arrOriginal;
    EXPECT_EQ(0, std::memcmp(&expectedData[0], arrCopy.data(), expectedData.size()));
}

TEST_F(ByteArrayTest, CopyAssign)
{
    std::string storedContent = "1234";
    ByteArray arrOriginal = ByteArray::fromString(storedContent);
    ByteArray arrCopy;
    arrCopy = arrOriginal;
    EXPECT_EQ(0, std::memcmp(storedContent.c_str(), arrCopy.data(), storedContent.size()));
}

TEST_F(ByteArrayTest, MoveAssign)
{
    std::string storedContent = "1234";
    ByteArray arrCopy;
    arrCopy = ByteArray::fromString(storedContent);
    EXPECT_EQ(0, std::memcmp(storedContent.c_str(), arrCopy.data(), storedContent.size()));
}


TEST_F(ByteArrayTest, fromStringConversion)
{
    std::string storedContent = "1234";
    ByteArray arr = ByteArray::fromString(storedContent);
    EXPECT_EQ(0, std::memcmp(storedContent.c_str(), arr.data(), storedContent.size()));
}

TEST_F(ByteArrayTest, hexConversion_correctHex)
{
    std::string storedContent = "0a2B3C4f9812";
    std::vector<std::byte> expectedContent = {std::byte{0x0A}, std::byte{0x2B}, std::byte{0x3C},
                                              std::byte{0x4F}, std::byte{0x98}, std::byte{0x12}};
    auto arr = ByteArray::fromHex(storedContent);
    EXPECT_TRUE(arr.hasValue());
    EXPECT_EQ(0, std::memcmp(&expectedContent[0], arr->data(), expectedContent.size()));
}

TEST_F(ByteArrayTest, hexConversion_correctHexWithSpaces)
{
    std::string storedContent = "0a 2B 3C 4f\t98\n12";
    std::vector<std::byte> expectedContent = {std::byte{0x0A}, std::byte{0x2B}, std::byte{0x3C},
                                              std::byte{0x4F}, std::byte{0x98}, std::byte{0x12}};
    auto arr = ByteArray::fromHex(storedContent);
    EXPECT_TRUE(arr.hasValue());
    EXPECT_EQ(0, std::memcmp(&expectedContent[0], arr->data(), expectedContent.size()));
}

TEST_F(ByteArrayTest, hexConversion_clippedHexExpectedErr)
{
    std::string storedContent = "0AC";
    auto arr = ByteArray::fromHex(storedContent);
    EXPECT_TRUE(arr.hasError());
}

TEST_F(ByteArrayTest, hexConversion_WrongSymbolExpectedErr)
{
    std::string storedContent = "0ACFL0";
    auto arr = ByteArray::fromHex(storedContent);
    EXPECT_TRUE(arr.hasError());
}

TEST_F(ByteArrayTest, hexConversion_specialCharacters)
{
    std::string storedContent = "0a 2B 3C 4f+98*12";
    std::vector<std::byte> expectedContent = {std::byte{0x0A}, std::byte{0x2B}, std::byte{0x3C},
                                              std::byte{0x4F}, std::byte{0x98}, std::byte{0x12}};
    auto arr = ByteArray::fromHex(storedContent, "+*");
    EXPECT_TRUE(arr.hasValue());
    EXPECT_EQ(0, std::memcmp(&expectedContent[0], arr->data(), expectedContent.size()));
}

TEST_F(ByteArrayTest, hexConversion_specialCharactersWrongSizeExpectedErr)
{
    std::string storedContent = "0a 2B 3C 4f+98**2";
    std::vector<std::byte> expectedContent = {std::byte{0x0A}, std::byte{0x2B}, std::byte{0x3C},
                                              std::byte{0x4F}, std::byte{0x98}, std::byte{0x12}};
    auto arr = ByteArray::fromHex(storedContent, "+*");
    EXPECT_TRUE(arr.hasError());
}

TEST_F(ByteArrayTest, CreateFromRange)
{
    std::vector<std::byte> content = {std::byte{0x0A}, std::byte{0x2B}, std::byte{0x3C},
                                      std::byte{0x4F}, std::byte{0x98}, std::byte{0x12}};
    ByteArray arr(content.cbegin(), content.cend());
    EXPECT_EQ(0, std::memcmp(&content[0], arr.data(), content.size()));
}

TEST_F(ByteArrayTest, createFromInitializeList)
{
    auto arr = ByteArray{std::byte{0x12}, std::byte{0x23}, std::byte{0x09}};
    const auto expected = std::vector<std::byte>{std::byte{0x12}, std::byte{0x23}, std::byte{0x09}};
    EXPECT_EQ(3, arr.size());
    EXPECT_EQ(0, std::memcmp(&expected[0], arr.data(), expected.size()));
}

TEST_F(ByteArrayTest, createFromInitializeListWith_2_elementsExpectedNoCompilationError)
{
    auto arr = ByteArray{std::byte{0x12}, std::byte{0x23}};
    const auto expected = std::vector<std::byte>{std::byte{0x12}, std::byte{0x23}};
    EXPECT_EQ(2, arr.size());
    EXPECT_EQ(0, std::memcmp(&expected[0], arr.data(), expected.size()));
}

TEST_F(ByteArrayTest, begin_end)
{
    std::vector<std::byte> content = {std::byte{0x0A}, std::byte{0x2B}, std::byte{0x3C},
                                      std::byte{0x4F}, std::byte{0x98}, std::byte{0x12}};
    ByteArray arr(content.cbegin(), content.cend());

    EXPECT_TRUE(std::equal(content.cbegin(), content.cend(), arr.cbegin()));
    EXPECT_TRUE(std::equal(content.cbegin(), content.cend(), arr.begin()));

    EXPECT_TRUE(std::equal(arr.cbegin(), arr.cend(), content.cbegin()));
    EXPECT_TRUE(std::equal(arr.begin(), arr.end(), content.cbegin()));
}

TEST_F(ByteArrayTest, mid_fromMiddle)
{
    std::vector<std::byte> content = {std::byte{0x0A}, std::byte{0x2B}, std::byte{0x3C},
                                      std::byte{0x4F}, std::byte{0x98}, std::byte{0x12}};
    ByteArray arr(content.cbegin(), content.cend());
    ByteArray fromMiddle = arr.mid(1, 2);

    EXPECT_EQ(0, std::memcmp(&content[1], fromMiddle.data(), 2));
}

TEST_F(ByteArrayTest, mid_defaultLength)
{
    std::vector<std::byte> content = {std::byte{0x0A}, std::byte{0x2B}, std::byte{0x3C},
                                      std::byte{0x4F}, std::byte{0x98}, std::byte{0x12}};
    ByteArray arr(content.cbegin(), content.cend());
    ByteArray fromMiddle = arr.mid(1);

    EXPECT_EQ(0, std::memcmp(&content[1], fromMiddle.data(), content.size() - 1));
}

TEST_F(ByteArrayTest, mid_lengthMoreThanSizeExceptionExpected)
{
    std::vector<std::byte> content = {std::byte{0x0A}, std::byte{0x2B}, std::byte{0x3C},
                                      std::byte{0x4F}, std::byte{0x98}, std::byte{0x12}};
    ByteArray arr(content.cbegin(), content.cend());

    try {
        ByteArray fromMiddle = arr.mid(100);
        FAIL() << "std::out_of_range exception is expected, method returns without error";
    } catch (std::out_of_range &) {
    } catch (...) {
        FAIL() << "Expected std::out_of_range, got other";
    }
}

TEST_F(ByteArrayTest, first_getSubArray)
{
    const auto arr = ByteArray::fromHex("0011BBFF");
    const auto first = arr->first(3);
    const auto expected = std::vector<std::byte>{std::byte{0x00}, std::byte{0x11}, std::byte{0xBB}};
    EXPECT_EQ(0, std::memcmp(&expected[0], first.data(), expected.size()));
    EXPECT_EQ(3, first.size());
}

TEST_F(ByteArrayTest, first_getBigSizeWholeArrayExpected)
{
    const auto arr = ByteArray::fromHex("0011BBFF");
    const auto first = arr->first(100);
    const auto expected =
        std::vector<std::byte>{std::byte{0x00}, std::byte{0x11}, std::byte{0xBB}, std::byte{0xFF}};
    EXPECT_EQ(0, std::memcmp(&expected[0], first.data(), expected.size()));
    EXPECT_EQ(4, first.size());
}

TEST_F(ByteArrayTest, first_forEmptyObjectEmptyExpected)
{
    const auto arr = ByteArray{};
    const auto first = arr.first(3);

    EXPECT_EQ(0, first.size());
}

TEST_F(ByteArrayTest, last_getSubArray)
{
    const auto arr = ByteArray::fromHex("0011BBFF");
    const auto last = arr->last(3);
    const auto expected = std::vector<std::byte>{std::byte{0x11}, std::byte{0xbb}, std::byte{0xff}};
    EXPECT_EQ(0, std::memcmp(&expected[0], last.data(), expected.size()));
    EXPECT_EQ(3, last.size());
}

TEST_F(ByteArrayTest, last_getBigSizeWholeArrayExpected)
{
    const auto arr = ByteArray::fromHex("0011BBFF");
    const auto last = arr->last(100);
    const auto expected =
        std::vector<std::byte>{std::byte{0x00}, std::byte{0x11}, std::byte{0xBB}, std::byte{0xFF}};
    EXPECT_EQ(0, std::memcmp(&expected[0], last.data(), expected.size()));
    EXPECT_EQ(4, last.size());
}

TEST_F(ByteArrayTest, last_forEmptyObjectEmptyResultExpected)
{
    const auto arr = ByteArray{};
    const auto last = arr.last(3);

    EXPECT_EQ(0, last.size());
}

TEST_F(ByteArrayTest, startsWith_correct_ByteArray)
{
    std::vector<std::byte> content = {std::byte{0x0A}, std::byte{0x2B}, std::byte{0x3C},
                                      std::byte{0x4F}, std::byte{0x98}, std::byte{0x12}};
    ByteArray arr(content.cbegin(), content.cend());

    ByteArray prefix(content.cbegin(), content.cbegin() + 2);

    EXPECT_TRUE(arr.startsWith(prefix));
}

TEST_F(ByteArrayTest, startsWith_empty_ByteArray)
{
    std::vector<std::byte> content = {std::byte{0x0A}, std::byte{0x2B}, std::byte{0x3C},
                                      std::byte{0x4F}, std::byte{0x98}, std::byte{0x12}};
    ByteArray arr(content.cbegin(), content.cend());

    ByteArray prefix;

    EXPECT_TRUE(arr.startsWith(prefix));
}

TEST_F(ByteArrayTest, endWith_correct_ByteArray)
{
    const auto original = ByteArray::fromHex("00 11 22 BB FF").value();
    const auto test = ByteArray::fromHex("11 22 BB FF").value();
    EXPECT_TRUE(original.endsWith(test));
}

TEST_F(ByteArrayTest, endWith_theSameObject)
{
    const auto original = ByteArray::fromHex("00 11 22 BB FF").value();
    const auto test = ByteArray::fromHex("00 11 22 BB FF").value();
    EXPECT_TRUE(original.endsWith(test));
}

TEST_F(ByteArrayTest, endWith_differentExpectExpectedFalse)
{
    const auto original = ByteArray::fromHex("00 11 22 BB FF").value();
    const auto test = ByteArray::fromHex("BB 1F").value();
    EXPECT_FALSE(original.endsWith(test));
}

TEST_F(ByteArrayTest, endWith_biggerObjectExpectFalse)
{
    const auto original = ByteArray::fromHex("00 11 22 BB FF").value();
    const auto test = ByteArray::fromHex("00 11 22 BB FF CC").value();
    EXPECT_FALSE(original.endsWith(test));
}

TEST_F(ByteArrayTest, endWith_emptyObjectAsArgumentTrueExpected)
{
    const auto original = ByteArray::fromHex("00 11 22 BB FF").value();
    const auto test = ByteArray{};
    EXPECT_TRUE(original.endsWith(test));
}

TEST_F(ByteArrayTest, contains_correctObject)
{
    const auto original = ByteArray::fromHex("00 11 22 BB FF").value();
    const auto test = ByteArray::fromHex("11 22").value();
    EXPECT_TRUE(original.contains(test));
}

TEST_F(ByteArrayTest, contains_objectWithSameContentExpectedTrue)
{
    const auto original = ByteArray::fromHex("00 11 22 BB FF").value();
    const auto test = ByteArray::fromHex("00 11 22 BB FF").value();
    EXPECT_TRUE(original.contains(test));
}

TEST_F(ByteArrayTest, contains_correctBytesFromTheEnd)
{
    const auto original = ByteArray::fromHex("00 11 22 BB FF").value();
    const auto test = ByteArray::fromHex("BB FF").value();
    EXPECT_TRUE(original.contains(test));
}

TEST_F(ByteArrayTest, contains_correctBytesFromTheBegging)
{
    const auto original = ByteArray::fromHex("00 11 22 BB FF").value();
    const auto test = ByteArray::fromHex("00 11").value();
    EXPECT_TRUE(original.contains(test));
}

TEST_F(ByteArrayTest, contains_largerObjectFalseExpected)
{
    const auto original = ByteArray::fromHex("00 11 22 BB FF").value();
    const auto test = ByteArray::fromHex("00 11 22 BB FF AA").value();
    EXPECT_FALSE(original.contains(test));
}

TEST_F(ByteArrayTest, contains_oneByteDifferentFalseExpected)
{
    const auto original = ByteArray::fromHex("00 11 22 BB FF").value();
    const auto test = ByteArray::fromHex("22 1B FF").value();
    EXPECT_FALSE(original.contains(test));
}

TEST_F(ByteArrayTest, contains_emptyObjectTrueExpected)
{
    const auto original = ByteArray::fromHex("00 11 22 BB FF").value();
    const auto test = ByteArray{};
    EXPECT_TRUE(original.contains(test));
}

TEST_F(ByteArrayTest, match_correctParameters)
{
    const auto original = ByteArray::fromHex("00 11 22 BB FF").value();
    const auto test = ByteArray::fromHex("22 BB").value();
    EXPECT_TRUE(original.matches(2, test));
}

TEST_F(ByteArrayTest, match_containedObjectWithWrongPositionFalseExpected)
{
    const auto original = ByteArray::fromHex("00 11 22 BB FF").value();
    const auto test = ByteArray::fromHex("22 BB").value();
    EXPECT_FALSE(original.matches(1, test));
}

TEST_F(ByteArrayTest, match_veryBigObjectFalseExpected)
{
    const auto original = ByteArray::fromHex("00 11 22 BB FF").value();
    const auto test = ByteArray::fromHex("11 22 BB FF AA").value();
    EXPECT_FALSE(original.matches(1, test));
}

TEST_F(ByteArrayTest, match_differentObjectFalseExpected)
{
    const auto original = ByteArray::fromHex("00 11 22 BB FF").value();
    const auto test = ByteArray::fromHex("11 22 1B").value();
    EXPECT_FALSE(original.matches(1, test));
}

TEST_F(ByteArrayTest, match_emptyTrueExpected)
{
    const auto original = ByteArray::fromHex("00 11 22 BB FF").value();
    const auto test = ByteArray{};
    EXPECT_TRUE(original.matches(1, test));
}

TEST_F(ByteArrayTest, match_wrongIndexFalseExpected)
{
    const auto original = ByteArray::fromHex("00 11 22 BB FF").value();
    const auto test = ByteArray{};
    EXPECT_FALSE(original.matches(100, test));
}

TEST_F(ByteArrayTest, comparing_twoEqualObjects)
{
    const auto original = ByteArray::fromHex("00 11 22 BB FF").value();
    const auto test = ByteArray::fromHex("00 11 22 BB FF").value();

    EXPECT_TRUE(original == test);
    EXPECT_FALSE(original != test);
}

TEST_F(ByteArrayTest, comparing_twoObjectsWithDifferentContentButSameSize)
{
    const auto original = ByteArray::fromHex("00 11 22 BB FF").value();
    const auto test = ByteArray::fromHex("00 11 22 1B FF").value();

    EXPECT_FALSE(original == test);
    EXPECT_TRUE(original != test);
}

TEST_F(ByteArrayTest, comparing_twoObjectsWithDifferentSize)
{
    const auto original = ByteArray::fromHex("00 11 22 BB FF").value();
    const auto test = ByteArray::fromHex("00 11 22 1B FF BB").value();

    EXPECT_FALSE(original == test);
    EXPECT_TRUE(original != test);
}

TEST_F(ByteArrayTest, isEmpty_size_forFilledObject)
{
    std::vector<std::byte> content = {std::byte{0x0A}, std::byte{0x2B}, std::byte{0x3C},
                                      std::byte{0x4F}, std::byte{0x98}, std::byte{0x12}};
    ByteArray arr(content.cbegin(), content.cend());

    EXPECT_FALSE(arr.isEmpty());
    EXPECT_EQ(6, arr.size());
}

TEST_F(ByteArrayTest, isEmpty_size_forEmptyObject)
{
    ByteArray arr;

    EXPECT_TRUE(arr.isEmpty());
    EXPECT_EQ(0, arr.size());
}

TEST_F(ByteArrayTest, indexAccess)
{
    std::vector<std::byte> content = {std::byte{0x0A}, std::byte{0x2B}, std::byte{0x3C}};
    ByteArray arr(content.cbegin(), content.cend());

    EXPECT_EQ(std::byte{0x0A}, arr[0]);
    EXPECT_EQ(std::byte{0x2B}, arr[1]);
    EXPECT_EQ(std::byte{0x3C}, arr[2]);

    EXPECT_EQ(std::byte{0x0A}, arr.at(0));
    EXPECT_EQ(std::byte{0x2B}, arr.at(1));
    EXPECT_EQ(std::byte{0x3C}, arr.at(2));

    const ByteArray &ref = arr;

    // check the same for const ref
    EXPECT_EQ(std::byte{0x0A}, ref[0]);
    EXPECT_EQ(std::byte{0x2B}, ref[1]);
    EXPECT_EQ(std::byte{0x3C}, ref[2]);

    EXPECT_EQ(std::byte{0x0A}, ref.at(0));
    EXPECT_EQ(std::byte{0x2B}, ref.at(1));
    EXPECT_EQ(std::byte{0x3C}, ref.at(2));
}

TEST_F(ByteArrayTest, indexAccess_modifyByte)
{
    std::vector<std::byte> content = {std::byte{0x0A}, std::byte{0x2B}, std::byte{0x3C},
                                      std::byte{0x4F}, std::byte{0x98}, std::byte{0x12}};
    std::vector<std::byte> expectedContent = {std::byte{0x0A}, std::byte{0xff}, // modified
                                              std::byte{0x3C}, std::byte{0x4F},
                                              std::byte{0x98}, std::byte{0x12}};
    ByteArray arr(content.cbegin(), content.cend());
    arr[1] = std::byte{0xff};

    EXPECT_EQ(std::byte{0xff}, arr[1]);

    EXPECT_EQ(0, std::memcmp(&expectedContent[0], arr.data(), expectedContent.size()));
}

TEST_F(ByteArrayTest, indexAccess_at_modifyByte)
{
    std::vector<std::byte> content = {std::byte{0x0A}, std::byte{0x2B}, std::byte{0x3C},
                                      std::byte{0x4F}, std::byte{0x98}, std::byte{0x12}};
    std::vector<std::byte> expectedContent = {std::byte{0x0A}, std::byte{0xff}, // modified
                                              std::byte{0x3C}, std::byte{0x4F},
                                              std::byte{0x98}, std::byte{0x12}};
    ByteArray arr(content.cbegin(), content.cend());
    arr.at(1) = std::byte{0xff};

    EXPECT_EQ(std::byte{0xff}, arr[1]);

    EXPECT_EQ(0, std::memcmp(&expectedContent[0], arr.data(), expectedContent.size()));
}


TEST_F(ByteArrayTest, indexAccess_at_outOfRangeExceptionExpected)
{
    std::vector<std::byte> content = {std::byte{0x0A}, std::byte{0x2B}, std::byte{0x3C},
                                      std::byte{0x4F}, std::byte{0x98}, std::byte{0x12}};
    ByteArray arr(content.cbegin(), content.cend());

    try {
        [[maybe_unused]] auto bt = arr.at(1000);
        FAIL() << "std::out_of_range exception is expected, method returns without error";
    } catch (std::out_of_range &) {
    } catch (...) {
        FAIL() << "Expected std::out_of_range, got other";
    }
}

TEST_F(ByteArrayTest, indexAccess_at_const_outOfRangeExceptionExpected)
{
    std::vector<std::byte> content = {std::byte{0x0A}, std::byte{0x2B}, std::byte{0x3C},
                                      std::byte{0x4F}, std::byte{0x98}, std::byte{0x12}};
    ByteArray arr(content.cbegin(), content.cend());
    const ByteArray &ref = arr;

    try {
        [[maybe_unused]] auto bt = ref.at(1000);
        FAIL() << "std::out_of_range exception is expected, method returns without error";
    } catch (std::out_of_range &) {
    } catch (...) {
        FAIL() << "Expected std::out_of_range, got other";
    }
}

TEST_F(ByteArrayTest, push_back)
{
    ByteArray arr;
    arr.push_back(std::byte{0xAA});
    arr.push_back(std::byte{0xCC});
    arr.push_back(std::byte{0xDD});
    const auto expected = std::vector<std::byte>{std::byte{0xAA}, std::byte{0xCC}, std::byte{0xDD}};
    EXPECT_EQ(arr.size(), 3);
    EXPECT_TRUE(std::equal(expected.cbegin(), expected.cend(), arr.cbegin(), arr.cend()));
}

TEST_F(ByteArrayTest, append)
{
    ByteArray arr;
    arr.append(std::byte{0xAA}).append(std::byte{0xCC}).append(std::byte{0xDD});
    const auto expected = std::vector<std::byte>{std::byte{0xAA}, std::byte{0xCC}, std::byte{0xDD}};
    EXPECT_EQ(arr.size(), 3);
    EXPECT_TRUE(std::equal(expected.cbegin(), expected.cend(), arr.cbegin(), arr.cend()));
}

TEST_F(ByteArrayTest, append_container)
{
    const auto otherContent = std::vector<std::byte>{std::byte{0x12}, std::byte{0x11}};
    ByteArray other{otherContent.cbegin(), otherContent.cend()};
    ByteArray arr;
    arr.append(other).append(other);
    const auto expected =
        std::vector<std::byte>{std::byte{0x12}, std::byte{0x11}, std::byte{0x12}, std::byte{0x11}};
    EXPECT_EQ(arr.size(), 4);
    EXPECT_TRUE(std::equal(expected.cbegin(), expected.cend(), arr.cbegin(), arr.cend()));
}

TEST_F(ByteArrayTest, insert_intoMiddle)
{
    ByteArray arr;
    arr.push_back(std::byte{0xAA});
    arr.push_back(std::byte{0xCC});
    const auto expected = std::vector<std::byte>{std::byte{0xAA}, std::byte{0xBB}, std::byte{0xCC}};
    auto insertedIter = arr.insert(std::next(arr.begin()), std::byte{0xBB});
    EXPECT_EQ(arr.size(), 3);
    EXPECT_TRUE(std::equal(expected.cbegin(), expected.cend(), arr.cbegin(), arr.cend()));
    EXPECT_EQ(*insertedIter, std::byte{0xBB});
}

TEST_F(ByteArrayTest, insert_intoTail)
{
    ByteArray arr;
    arr.push_back(std::byte{0xAA});
    arr.push_back(std::byte{0xCC});
    const auto expected = std::vector<std::byte>{std::byte{0xAA}, std::byte{0xCC}, std::byte{0xBB}};
    auto insertedIter = arr.insert(arr.end(), std::byte{0xBB});
    EXPECT_EQ(arr.size(), 3);
    EXPECT_TRUE(std::equal(expected.cbegin(), expected.cend(), arr.cbegin(), arr.cend()));
    EXPECT_EQ(*insertedIter, std::byte{0xBB});
}

TEST_F(ByteArrayTest, insert_intoHead)
{
    ByteArray arr;
    arr.push_back(std::byte{0xAA});
    arr.push_back(std::byte{0xCC});
    const auto expected = std::vector<std::byte>{std::byte{0xBB}, std::byte{0xAA}, std::byte{0xCC}};
    auto insertedIter = arr.insert(arr.begin(), std::byte{0xBB});
    EXPECT_EQ(arr.size(), 3);
    EXPECT_TRUE(std::equal(expected.cbegin(), expected.cend(), arr.cbegin(), arr.cend()));
    EXPECT_EQ(*insertedIter, std::byte{0xBB});
}

TEST_F(ByteArrayTest, insert_pointWith_ConstIterator)
{
    ByteArray arr;
    arr.push_back(std::byte{0xAA});
    arr.push_back(std::byte{0xCC});
    const auto expected = std::vector<std::byte>{std::byte{0xAA}, std::byte{0xBB}, std::byte{0xCC}};
    auto insertedIter = arr.insert(std::next(arr.cbegin()), std::byte{0xBB});
    EXPECT_EQ(arr.size(), 3);
    EXPECT_TRUE(std::equal(expected.cbegin(), expected.cend(), arr.cbegin(), arr.cend()));
    EXPECT_EQ(*insertedIter, std::byte{0xBB});
}

TEST_F(ByteArrayTest, std_back_inserter_pushBackBytes)
{
    ByteArray arr;
    arr.push_back(std::byte{0x12});
    arr.push_back(std::byte{0xAC});

    auto insertContent = std::vector<std::byte>{std::byte{0xCC}, std::byte{0xFF}, std::byte{0xAA}};
    std::copy(insertContent.cbegin(), insertContent.cend(), std::back_inserter(arr));
    auto expected = std::vector<std::byte>{std::byte{0x12}, std::byte{0xAC}, std::byte{0xCC},
                                           std::byte{0xFF}, std::byte{0xAA}};
    EXPECT_EQ(arr.size(), expected.size());
    EXPECT_TRUE(std::equal(expected.cbegin(), expected.cend(), arr.cbegin(), arr.cend()));
}

TEST_F(ByteArrayTest, std_inserter_insertIntoMiddle)
{
    ByteArray arr;
    arr.push_back(std::byte{0x12});
    arr.push_back(std::byte{0xAC});

    auto insertContent = std::vector<std::byte>{std::byte{0xCC}, std::byte{0xFF}, std::byte{0xAA}};
    std::copy(insertContent.cbegin(), insertContent.cend(),
              std::inserter(arr, std::next(arr.begin())));
    auto expected = std::vector<std::byte>{std::byte{0x12}, std::byte{0xCC}, std::byte{0xFF},
                                           std::byte{0xAA}, std::byte{0xAC}};
    EXPECT_EQ(arr.size(), expected.size());
    EXPECT_TRUE(std::equal(expected.cbegin(), expected.cend(), arr.cbegin(), arr.cend()));
}

TEST_F(ByteArrayTest, insert_intoMiddleWithPos)
{
    ByteArray arr;
    arr.insert(0, std::byte{0x11})   // arr = {0x11}
        .insert(1, std::byte{0xAA})  // arr = {0x11, 0xAA}
        .insert(1, std::byte{0xBB}); // arr = {0x11, 0xBB, 0xAA}
    auto expected = std::vector<std::byte>{std::byte{0x11}, std::byte{0xBB}, std::byte{0xAA}};
    EXPECT_EQ(arr.size(), expected.size());
    EXPECT_TRUE(std::equal(expected.cbegin(), expected.cend(), arr.cbegin(), arr.cend()));
}

TEST_F(ByteArrayTest, insert_ByteArray_inFront)
{
    const auto otherContent =
        std::vector<std::byte>{std::byte{0x00}, std::byte{0x11}, std::byte{0x22}};
    ByteArray other{otherContent.cbegin(), otherContent.cend()};
    ByteArray arr;
    arr.push_back(std::byte{0xBB});
    arr.push_back(std::byte{0xCC});

    const auto expected = std::vector<std::byte>{std::byte{0x00}, std::byte{0x11}, std::byte{0x22},
                                                 std::byte{0xBB}, std::byte{0xCC}};

    arr.insert(0, other);
    EXPECT_EQ(arr.size(), expected.size());
    EXPECT_TRUE(std::equal(expected.cbegin(), expected.cend(), arr.cbegin(), arr.cend()));
}

TEST_F(ByteArrayTest, insert_ByteArray_intoBack)
{
    const auto otherContent =
        std::vector<std::byte>{std::byte{0x00}, std::byte{0x11}, std::byte{0x22}};
    ByteArray other{otherContent.cbegin(), otherContent.cend()};
    ByteArray arr;
    arr.push_back(std::byte{0xBB});
    arr.push_back(std::byte{0xCC});

    const auto expected = std::vector<std::byte>{std::byte{0xBB}, std::byte{0xCC}, std::byte{0x00},
                                                 std::byte{0x11}, std::byte{0x22}};

    arr.insert(2, other);
    EXPECT_EQ(arr.size(), expected.size());
    EXPECT_TRUE(std::equal(expected.cbegin(), expected.cend(), arr.cbegin(), arr.cend()));
}

TEST_F(ByteArrayTest, insert_ByteArray_inMid)
{
    const auto otherContent =
        std::vector<std::byte>{std::byte{0x00}, std::byte{0x11}, std::byte{0x22}};
    ByteArray other{otherContent.cbegin(), otherContent.cend()};
    ByteArray arr;
    arr.push_back(std::byte{0xBB});
    arr.push_back(std::byte{0xCC});

    const auto expected = std::vector<std::byte>{std::byte{0xBB}, std::byte{0x00}, std::byte{0x11},
                                                 std::byte{0x22}, std::byte{0xCC}};

    arr.insert(1, other);
    EXPECT_EQ(arr.size(), expected.size());
    EXPECT_TRUE(std::equal(expected.cbegin(), expected.cend(), arr.cbegin(), arr.cend()));
}

TEST_F(ByteArrayTest, popBack_oneElement)
{
    ByteArray arr = ByteArray::fromHex("01AB").value();
    EXPECT_EQ(2, arr.size());
    arr.popBack();
    EXPECT_EQ(1, arr.size());
    EXPECT_EQ(std::byte{0x01}, arr[0]);
}

TEST_F(ByteArrayTest, clear_wholeArray)
{
    ByteArray arr = ByteArray::fromHex("01AB").value();
    EXPECT_EQ(2, arr.size());
    arr.clear();
    EXPECT_EQ(0, arr.size());
}

TEST_F(ByteArrayTest, erase_startingPosition_1_withCount_2)
{
    ByteArray arr = ByteArray::fromHex("01ABCD02EF").value();
    const auto expected = std::vector<std::byte>{std::byte{0x01}, std::byte{0x02}, std::byte{0xEF}};
    arr.erase(1, 2);
    EXPECT_EQ(3, arr.size());
    EXPECT_EQ(0, std::memcmp(&expected[0], arr.data(), expected.size()));
}

TEST_F(ByteArrayTest, erase_veryBigIndexExceptionExpected)
{
    ByteArray arr = ByteArray::fromHex("01ABCD02EF").value();
    EXPECT_EQ(arr.size(), 5);

    try {
        arr.erase(6);
        FAIL() << "std::out_of_range exception is expected, method returns without error";
    } catch (std::out_of_range &) {
    } catch (...) {
        FAIL() << "Expected std::out_of_range, got other";
    }
}

TEST_F(ByteArrayTest, erase_fromPosition_2_tillEnd)
{
    ByteArray arr = ByteArray::fromHex("01ABCD02EF").value();
    const auto expected = std::vector<std::byte>{std::byte{0x01}, std::byte{0xAB}};
    arr.erase(2);
    EXPECT_EQ(2, arr.size());
    EXPECT_EQ(0, std::memcmp(&expected[0], arr.data(), expected.size()));
}

TEST_F(ByteArrayTest, erase_Iterator)
{
    ByteArray arr = ByteArray::fromHex("01ABCD02EF").value();
    const auto expected =
        std::vector<std::byte>{std::byte{0x01}, std::byte{0xCD}, std::byte{0x02}, std::byte{0xEF}};
    ByteArray::Iterator removeIter = std::find(arr.begin(), arr.end(), std::byte{0xAB});

    auto nextAfterRemoved = arr.erase(removeIter);

    EXPECT_EQ(*nextAfterRemoved, std::byte{0xCD});

    EXPECT_EQ(0, std::memcmp(&expected[0], arr.data(), expected.size()));
}

TEST_F(ByteArrayTest, erase_ConstIterator)
{
    ByteArray arr = ByteArray::fromHex("01ABCD02EF").value();
    const auto expected =
        std::vector<std::byte>{std::byte{0x01}, std::byte{0xCD}, std::byte{0x02}, std::byte{0xEF}};
    ByteArray::ConstIterator removeIter = std::find(arr.cbegin(), arr.cend(), std::byte{0xAB});

    auto nextAfterRemoved = arr.erase(removeIter);

    EXPECT_EQ(*nextAfterRemoved, std::byte{0xCD});

    EXPECT_EQ(0, std::memcmp(&expected[0], arr.data(), expected.size()));
}

TEST_F(ByteArrayTest, erase_betweenSecondEndFourth_Iterator)
{
    ByteArray arr = ByteArray::fromHex("01ABCD02EF").value();
    const auto expected = std::vector<std::byte>{std::byte{0x01}, std::byte{0x02}, std::byte{0xEF}};
    ByteArray::Iterator first = arr.begin() + 1;
    ByteArray::Iterator last = arr.begin() + 3;
    auto nextAfterRemoved = arr.erase(first, last);

    EXPECT_EQ(*nextAfterRemoved, std::byte{0x02});
    EXPECT_EQ(0, std::memcmp(&expected[0], arr.data(), expected.size()));
}

TEST_F(ByteArrayTest, erase_betweenSecondEndFourth_ConstIterator)
{
    ByteArray arr = ByteArray::fromHex("01ABCD02EF").value();
    const auto expected = std::vector<std::byte>{std::byte{0x01}, std::byte{0x02}, std::byte{0xEF}};
    ByteArray::ConstIterator first = arr.cbegin() + 1;
    ByteArray::ConstIterator last = arr.cbegin() + 3;
    auto nextAfterRemoved = arr.erase(first, last);

    EXPECT_EQ(*nextAfterRemoved, std::byte{0x02});
    EXPECT_EQ(0, std::memcmp(&expected[0], arr.data(), expected.size()));
}

TEST_F(ByteArrayTest, reserve_and_capacity)
{
    ByteArray arr = ByteArray::fromHex("001122").value();
    const auto expected = std::vector<std::byte>{std::byte{0x00}, std::byte{0x11}, std::byte{0x22}};
    arr.reserve(100);
    EXPECT_EQ(100, arr.capacity());
    EXPECT_EQ(0, std::memcmp(&expected[0], arr.data(), expected.size()));
}

TEST_F(ByteArrayTest, resize_reduce_the_size)
{
    ByteArray arr = ByteArray::fromHex("0011223344").value();
    const auto expected = std::vector<std::byte>{std::byte{0x00}, std::byte{0x11}, std::byte{0x22}};
    arr.resize(3);
    EXPECT_EQ(3, arr.size());
    EXPECT_EQ(0, std::memcmp(&expected[0], arr.data(), expected.size()));
}

TEST_F(ByteArrayTest, resize_extend_with_0)
{
    ByteArray arr = ByteArray::fromHex("001122").value();
    const auto expected = std::vector<std::byte>{std::byte{0x00}, std::byte{0x11}, std::byte{0x22},
                                                 std::byte{0x00}, std::byte{0x00}};
    arr.resize(5);
    EXPECT_EQ(5, arr.size());
    EXPECT_EQ(0, std::memcmp(&expected[0], arr.data(), expected.size()));
}

TEST_F(ByteArrayTest, resize_extend_with_BB)
{
    ByteArray arr = ByteArray::fromHex("001122").value();
    const auto expected = std::vector<std::byte>{std::byte{0x00}, std::byte{0x11}, std::byte{0x22},
                                                 std::byte{0xBB}, std::byte{0xBB}};
    arr.resize(5, std::byte{0xBB});
    EXPECT_EQ(5, arr.size());
    EXPECT_EQ(0, std::memcmp(&expected[0], arr.data(), expected.size()));
}

TEST_F(ByteArrayTest, swap)
{
    ByteArray firstArray = ByteArray::fromHex("001122").value();
    ByteArray secondArray = ByteArray::fromHex("BBAA").value();
    const auto firstExpected = std::vector<std::byte>{std::byte{0xBB}, std::byte{0xAA}};
    const auto secondExpected =
        std::vector<std::byte>{std::byte{0x00}, std::byte{0x11}, std::byte{0x22}};

    firstArray.swap(secondArray);

    EXPECT_EQ(2, firstArray.size());
    EXPECT_EQ(3, secondArray.size());
    EXPECT_EQ(0, std::memcmp(&firstExpected[0], firstArray.data(), firstExpected.size()));
    EXPECT_EQ(0, std::memcmp(&secondExpected[0], secondArray.data(), secondExpected.size()));
}

TEST_F(ByteArrayTest, reinterpret_readFromEmpty_emptyOptionalExpected)
{
    ByteArray arr;
    auto res = arr.reinterpret<std::uint16_t>(0);
    EXPECT_FALSE(res.has_value());
}

TEST_F(ByteArrayTest, reinterpret_readIndexOutOfRange_emptyOptionalExpected)
{
    ByteArray arr = ByteArray::fromHex("00 AA BB").value();
    auto res = arr.reinterpret<std::uint16_t>(2);
    EXPECT_FALSE(res.has_value());
}

TEST_F(ByteArrayTest, reinterpret_readCorrect_char)
{
    auto arr = ByteArray::fromHex("41 61 66 7A").value();
    auto value = arr.reinterpret<char>(3);
    EXPECT_TRUE(value.has_value());
    EXPECT_EQ('z', value.value());
}

TEST_F(ByteArrayTest, reinterpret_readCorrect_uint8_t)
{
    auto arr = ByteArray::fromHex("AA 00 01 Bc FF").value();
    auto value = arr.reinterpret<std::uint8_t>(3);
    EXPECT_TRUE(value.has_value());
    EXPECT_EQ(0xBC, value.value());
}

TEST_F(ByteArrayTest, reinterpret_readCorrect_uint16_t)
{
    auto arr = ByteArray::fromHex("AA 00 01 Bc").value();
    auto value = arr.reinterpret<std::uint16_t>(1);
    EXPECT_TRUE(value.has_value());

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    EXPECT_EQ(0x0100, value.value());
#else
    EXPECT_EQ(0x0001, value.value());
#endif
}

TEST_F(ByteArrayTest, reinterpret_readCorrect_uint32_t)
{
    auto arr = ByteArray::fromHex("AA 00 01 Bc FF").value();
    auto value = arr.reinterpret<std::uint32_t>(1);
    EXPECT_TRUE(value.has_value());

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    EXPECT_EQ(0xFF'BC'01'00, value.value());
#else
    EXPECT_EQ(0x0001BCFF, value.value());
#endif
}

TEST_F(ByteArrayTest, reinterpret_readCorrect_uint64_t)
{
    auto arr = ByteArray::fromHex("AA 00 01 Bc FF AA BB CC 00").value();
    auto value = arr.reinterpret<std::uint64_t>(1);
    EXPECT_TRUE(value.has_value());

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    EXPECT_EQ(0x00'CC'BB'AA'FF'BC'01'00, value.value());
#else
    EXPECT_EQ(0x00'01'Bc'FF'AA'BB'CC'00, value.value());
#endif
}

TEST_F(ByteArrayTest, insertAsBytes__writeValueToTheEmpty_char)
{
    auto arr = ByteArray{};
    char value = 'N';
    arr.insertAsBytes(0, value);
    auto expected = std::vector<std::byte>{std::byte{0x4E}};
    EXPECT_EQ(1, arr.size());
    EXPECT_TRUE(std::equal(expected.cbegin(), expected.cend(), arr.cbegin(), arr.cend()));
}


TEST_F(ByteArrayTest, insertAsBytes_writeValueToTheEmpty_uint8_t)
{
    auto arr = ByteArray{};
    std::uint8_t value = 0x01;

    arr.insertAsBytes(0, value);
    auto expected = std::vector<std::byte>{std::byte{0x01}};
    EXPECT_EQ(1, arr.size());
    EXPECT_TRUE(std::equal(expected.cbegin(), expected.cend(), arr.cbegin(), arr.cend()));
}

TEST_F(ByteArrayTest, insertAsBytes_writeValueToTheEmpty_uint16_t)
{
    auto arr = ByteArray{};
    std::uint16_t value = 0x01BC;

    arr.insertAsBytes(0, value);
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    auto expected = std::vector<std::byte>{std::byte{0xBC}, std::byte{0x01}};
#else
    auto expected = std::vector<std::byte>{std::byte{0x01}, std::byte{0xBC}};
#endif
    EXPECT_EQ(2, arr.size());
    EXPECT_TRUE(std::equal(expected.cbegin(), expected.cend(), arr.cbegin(), arr.cend()));
}

TEST_F(ByteArrayTest, insertAsBytes_writeValueToTheEmpty_uint32_t)
{
    auto arr = ByteArray{};
    std::uint32_t value = 0x00'FF'01'BC;

    arr.insertAsBytes(0, value);
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    auto expected =
        std::vector<std::byte>{std::byte{0xBC}, std::byte{0x01}, std::byte{0xFF}, std::byte{0x00}};
#else
    auto expected =
        std::vector<std::byte>{std::byte{0x00}, std::byte{0xFF}, std::byte{0x01}, std::byte{0xBC}};
#endif
    EXPECT_EQ(4, arr.size());
    EXPECT_TRUE(std::equal(expected.cbegin(), expected.cend(), arr.cbegin(), arr.cend()));
}

TEST_F(ByteArrayTest, insertAsBytes_writeValueToTheEmpty_uint64_t)
{
    auto arr = ByteArray{};
    std::uint64_t value = 0x00'02'01'00'FF'01'BC'1A;

    arr.insertAsBytes(0, value);
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    auto expected =
        std::vector<std::byte>{std::byte{0x1A}, std::byte{0xBC}, std::byte{0x01}, std::byte{0xFF},
                               std::byte{0x00}, std::byte{0x01}, std::byte{0x02}, std::byte{0x00}};
#else
    auto expected =
        std::vector<std::byte>{std::byte{0x00}, std::byte{0x02}, std::byte{0x01}, std::byte{0x00},
                               std::byte{0xFF}, std::byte{0x01}, std::byte{0xBC}, std::byte{0x1A}};
#endif
    EXPECT_EQ(8, arr.size());
    EXPECT_TRUE(std::equal(expected.cbegin(), expected.cend(), arr.cbegin(), arr.cend()));
}

TEST_F(ByteArrayTest, insertAsBytes_writeValueToTheMid_uint16_t)
{
    auto arr = ByteArray::fromHex("00 01 BB").value();
    std::uint16_t value = 0x01BC;

    arr.insertAsBytes(1, value);
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    auto expected = std::vector<std::byte>{std::byte{0x00}, std::byte{0xBC}, std::byte{0x01},
                                           std::byte{0x01}, std::byte{0xBB}};
#else
    auto expected = std::vector<std::byte>{std::byte{0x00}, std::byte{0x01}, std::byte{0xBC},
                                           std::byte{0x01}, std::byte{0xBB}};
#endif
    EXPECT_EQ(5, arr.size());
    EXPECT_TRUE(std::equal(expected.cbegin(), expected.cend(), arr.cbegin(), arr.cend()));
}

TEST_F(ByteArrayTest, insertAsBytes_writeValueToTheEnd_uint16_t)
{
    auto arr = ByteArray::fromHex("00 01 BB").value();
    std::uint16_t value = 0x01BC;

    arr.insertAsBytes(arr.size(), value);
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    auto expected = std::vector<std::byte>{std::byte{0x00}, std::byte{0x01}, std::byte{0xBB},
                                           std::byte{0xBC}, std::byte{0x01}};
#else
    auto expected = std::vector<std::byte>{std::byte{0x00}, std::byte{0x01}, std::byte{0xBB},
                                           std::byte{0x01}, std::byte{0xBC}};
#endif
    EXPECT_EQ(5, arr.size());
    EXPECT_TRUE(std::equal(expected.cbegin(), expected.cend(), arr.cbegin(), arr.cend()));
}

TEST_F(ByteArrayTest, insertAsBytes_writeValueToTheFont_uint16_t)
{
    auto arr = ByteArray::fromHex("00 01 BB").value();
    std::uint16_t value = 0x01BC;

    arr.insertAsBytes(0, value);
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    auto expected = std::vector<std::byte>{std::byte{0xBC}, std::byte{0x01}, std::byte{0x00},
                                           std::byte{0x01}, std::byte{0xBB}};
#else
    auto expected = std::vector<std::byte>{std::byte{0x01}, std::byte{0xBC}, std::byte{0x00},
                                           std::byte{0x01}, std::byte{0xBB}};
#endif
    EXPECT_EQ(5, arr.size());
    EXPECT_TRUE(std::equal(expected.cbegin(), expected.cend(), arr.cbegin(), arr.cend()));
}

TEST_F(ByteArrayTest, insertAsBytes_chainingTwo_uint16_t)
{
    std::uint16_t one = 0x0011;
    std::uint16_t two = 0x0022;
    ByteArray arr;
    arr.insertAsBytes(arr.size(), one)   // content is {0x11, 0x00}
        .insertAsBytes(arr.size(), two); // content is {0x11, 0x00, 0x22, 0x00}
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    auto expected =
        std::vector<std::byte>{std::byte{0x11}, std::byte{0x00}, std::byte{0x22}, std::byte{0x00}};
#else
    auto expected =
        std::vector<std::byte>{std::byte{0x00}, std::byte{0x11}, std::byte{0x00}, std::byte{0x22}};
#endif
    EXPECT_EQ(4, arr.size());
    EXPECT_TRUE(std::equal(expected.cbegin(), expected.cend(), arr.cbegin(), arr.cend()));
}

TEST_F(ByteArrayTest, insertAsBytes_writeToWrongPositionExceptionExpected)
{
    ByteArray arr = ByteArray::fromHex("00 BB fF").value();
    try {
        arr.insertAsBytes(4, 4);
        FAIL() << "std::out_of_range exception is expected, method returns without error";
    } catch (std::out_of_range &) {
        SUCCEED();
    } catch (...) {
        FAIL() << "std::out_of_range exception is expected, other one is thrown";
    }
}

TEST_F(ByteArrayTest, literal_correctOne_with_quotes)
{
    auto arr = 0x00'01'AB'FF'CC'DD_hex;
    const auto expected = std::vector<std::byte>{std::byte{0x00}, std::byte{0x01}, std::byte{0xAB},
                                                 std::byte{0xFF}, std::byte{0xCC}, std::byte{0xDD}};
    EXPECT_EQ(6, arr.size());
    EXPECT_TRUE(std::equal(expected.cbegin(), expected.cend(), arr.cbegin(), arr.cend()));
}

TEST_F(ByteArrayTest, literal_correctOne_without_quotes)
{
    auto arr = 0x0001ABFFCCDD_hex;
    const auto expected = std::vector<std::byte>{std::byte{0x00}, std::byte{0x01}, std::byte{0xAB},
                                                 std::byte{0xFF}, std::byte{0xCC}, std::byte{0xDD}};
    EXPECT_EQ(6, arr.size());
    EXPECT_TRUE(std::equal(expected.cbegin(), expected.cend(), arr.cbegin(), arr.cend()));
}

TEST_F(ByteArrayTest, literal_correctOneBig_X)
{
    auto arr = 0X0001ABFFCCDD_hex;
    const auto expected = std::vector<std::byte>{std::byte{0x00}, std::byte{0x01}, std::byte{0xAB},
                                                 std::byte{0xFF}, std::byte{0xCC}, std::byte{0xDD}};
    EXPECT_EQ(6, arr.size());
    EXPECT_TRUE(std::equal(expected.cbegin(), expected.cend(), arr.cbegin(), arr.cend()));
}

TEST_F(ByteArrayTest, literal_longSequenceConversion)
{
    auto arr =
        0X00'01'AB'FF'CC'DD'FF'01'AB'FF'CC'DD'FF'01'AB'FF'CC'DD'FF'01'AB'FF'CC'DD'FF'01'AB'FF'CC'DD'FF'01'AB'FF'CC'DD'FF'01'AB'FF'CC'DD'FF'01'AB'FF'CC'DD'FF'01'AB'FF'CC'DD'FF'01'AB'FF'CC'DD'FF'01'AB'FF'CC'DD'FF'01'AB'FF'CC'DD'FF'01'AB'FF'CC'DD'FF'01'AB'FF'CC'DD'FF'01'AB'FF'CC'DD'FF'01'AB'FF'CC'DD'FF'00'01'AB'FF'CC'DD'FF'01'AB'FF'CC'DD'FF'01'AB'FF'CC'DD'FF'01'AB'FF'CC'DD'FF'01'AB'FF'CC'DD'FF'01'AB'FF'CC'DD'FF'01'AB'FF'CC'DD'FF'01'AB'FF'CC'DD'FF'01'AB'FF'CC'DD'FF'01'AB'FF'CC'DD'FF'01'AB'FF'CC'DD'FF'01'AB'FF'CC'DD'FF'01'AB'FF'CC'DD'FF'01'AB'FF'CC'DD'FF'01'AB'FF'CC'DD'FF'01'AB'FF'CC'DD'FF_hex;
    const auto expected = std::vector<std::byte>{
        std::byte{0x00}, std::byte{0x01}, std::byte{0xAB}, std::byte{0xFF}, std::byte{0xCC},
        std::byte{0xDD}, std::byte{0xFF}, std::byte{0x01}, std::byte{0xAB}, std::byte{0xFF},
        std::byte{0xCC}, std::byte{0xDD}, std::byte{0xFF}, std::byte{0x01}, std::byte{0xAB},
        std::byte{0xFF}, std::byte{0xCC}, std::byte{0xDD}, std::byte{0xFF}, std::byte{0x01},
        std::byte{0xAB}, std::byte{0xFF}, std::byte{0xCC}, std::byte{0xDD}, std::byte{0xFF},
        std::byte{0x01}, std::byte{0xAB}, std::byte{0xFF}, std::byte{0xCC}, std::byte{0xDD},
        std::byte{0xFF}, std::byte{0x01}, std::byte{0xAB}, std::byte{0xFF}, std::byte{0xCC},
        std::byte{0xDD}, std::byte{0xFF}, std::byte{0x01}, std::byte{0xAB}, std::byte{0xFF},
        std::byte{0xCC}, std::byte{0xDD}, std::byte{0xFF}, std::byte{0x01}, std::byte{0xAB},
        std::byte{0xFF}, std::byte{0xCC}, std::byte{0xDD}, std::byte{0xFF}, std::byte{0x01},
        std::byte{0xAB}, std::byte{0xFF}, std::byte{0xCC}, std::byte{0xDD}, std::byte{0xFF},
        std::byte{0x01}, std::byte{0xAB}, std::byte{0xFF}, std::byte{0xCC}, std::byte{0xDD},
        std::byte{0xFF}, std::byte{0x01}, std::byte{0xAB}, std::byte{0xFF}, std::byte{0xCC},
        std::byte{0xDD}, std::byte{0xFF}, std::byte{0x01}, std::byte{0xAB}, std::byte{0xFF},
        std::byte{0xCC}, std::byte{0xDD}, std::byte{0xFF}, std::byte{0x01}, std::byte{0xAB},
        std::byte{0xFF}, std::byte{0xCC}, std::byte{0xDD}, std::byte{0xFF}, std::byte{0x01},
        std::byte{0xAB}, std::byte{0xFF}, std::byte{0xCC}, std::byte{0xDD}, std::byte{0xFF},
        std::byte{0x01}, std::byte{0xAB}, std::byte{0xFF}, std::byte{0xCC}, std::byte{0xDD},
        std::byte{0xFF}, std::byte{0x01}, std::byte{0xAB}, std::byte{0xFF}, std::byte{0xCC},
        std::byte{0xDD}, std::byte{0xFF}, std::byte{0x00}, std::byte{0x01}, std::byte{0xAB},
        std::byte{0xFF}, std::byte{0xCC}, std::byte{0xDD}, std::byte{0xFF}, std::byte{0x01},
        std::byte{0xAB}, std::byte{0xFF}, std::byte{0xCC}, std::byte{0xDD}, std::byte{0xFF},
        std::byte{0x01}, std::byte{0xAB}, std::byte{0xFF}, std::byte{0xCC}, std::byte{0xDD},
        std::byte{0xFF}, std::byte{0x01}, std::byte{0xAB}, std::byte{0xFF}, std::byte{0xCC},
        std::byte{0xDD}, std::byte{0xFF}, std::byte{0x01}, std::byte{0xAB}, std::byte{0xFF},
        std::byte{0xCC}, std::byte{0xDD}, std::byte{0xFF}, std::byte{0x01}, std::byte{0xAB},
        std::byte{0xFF}, std::byte{0xCC}, std::byte{0xDD}, std::byte{0xFF}, std::byte{0x01},
        std::byte{0xAB}, std::byte{0xFF}, std::byte{0xCC}, std::byte{0xDD}, std::byte{0xFF},
        std::byte{0x01}, std::byte{0xAB}, std::byte{0xFF}, std::byte{0xCC}, std::byte{0xDD},
        std::byte{0xFF}, std::byte{0x01}, std::byte{0xAB}, std::byte{0xFF}, std::byte{0xCC},
        std::byte{0xDD}, std::byte{0xFF}, std::byte{0x01}, std::byte{0xAB}, std::byte{0xFF},
        std::byte{0xCC}, std::byte{0xDD}, std::byte{0xFF}, std::byte{0x01}, std::byte{0xAB},
        std::byte{0xFF}, std::byte{0xCC}, std::byte{0xDD}, std::byte{0xFF}, std::byte{0x01},
        std::byte{0xAB}, std::byte{0xFF}, std::byte{0xCC}, std::byte{0xDD}, std::byte{0xFF},
        std::byte{0x01}, std::byte{0xAB}, std::byte{0xFF}, std::byte{0xCC}, std::byte{0xDD},
        std::byte{0xFF}, std::byte{0x01}, std::byte{0xAB}, std::byte{0xFF}, std::byte{0xCC},
        std::byte{0xDD}, std::byte{0xFF}, std::byte{0x01}, std::byte{0xAB}, std::byte{0xFF},
        std::byte{0xCC}, std::byte{0xDD}, std::byte{0xFF}, std::byte{0x01}, std::byte{0xAB},
        std::byte{0xFF}, std::byte{0xCC}, std::byte{0xDD}, std::byte{0xFF}};
    EXPECT_EQ(expected.size(), arr.size());
    EXPECT_TRUE(std::equal(expected.cbegin(), expected.cend(), arr.cbegin(), arr.cend()));
}

TEST_F(ByteArrayTest, fromBase64_positive)
{
    std::vector<std::pair<std::string, ByteArray>> data{// from RFC 4648:
                                                        {"", {}},
                                                        {"Zg==", /* "f" */ 0X66_hex},
                                                        {"Zm8=", /* "fo" */ 0X66'6f_hex},
                                                        {"Zm9v", /* "foo" */ 0X66'6f'6f_hex},
                                                        {"Zm9vYg==",
                                                         /* "foob" */ 0X66'6f'6f'62_hex},
                                                        {"Zm9vYmE=",
                                                         /* "fooba" */ 0X66'6f'6f'62'61_hex},
                                                        {"Zm9vYmFy",
                                                         /* "foobar" */ 0X66'6f'6f'62'61'72_hex},
                                                        // without padding:
                                                        {"Zg", 0X66_hex},
                                                        {"Zm8", 0X66'6f_hex},
                                                        {"Zm9vYg", 0X66'6f'6f'62_hex},
                                                        {"Zm9vYmE", 0X66'6f'6f'62'61_hex},
                                                        // complete input alphabet:
                                                        {AsciiBase64, asciiByteArray()}};

    for (const auto &[input, expectedOutput] : data) {
        const auto result = ByteArray::fromBase64(input);
        EXPECT_TRUE(result.hasValue());
        EXPECT_EQ(*result, expectedOutput);
    }
}

TEST_F(ByteArrayTest, fromBase64_negative)
{
    std::vector<std::string> data{
        // Illegal characters discovered by boost's base64_from_binary:
        "KABOOM!",
        "Z$9vYmFy",
        // Illegal characters not discovered by base64_from_binary:
        "==9vYmFy",
        "9==vYmFy",
        // Illegal padding:
        "Zm9vYg===",
        "Zm9vYmFy==",
        "Zm9vYmFy=",
        "Zg=",
        // from tst_qbytearray.cpp (https://bit.ly/3UlYtUZ)
        "  ",
        "MQ=",
        "MTI       ",
        "M=TIz",
        "MTI zN A ",
        "Cg@",
        "======YQo=",
        "Y\nWIK ",
        "YWJjCg=",
        "YWJ\1j\x9cZAo=",
        "YW JjZ\n G\tUK",
        "YWJjZGVmCg=",
        "YWJ\rjZGVmZwo",
        "YWJjZGVmZ2gK====",
        "Zm9vAGJhcg=",
        "ZtFvb5x 0YXI",
        "IgAAAAAAACI ",
    };

    for (const auto &input : data) {
        const auto result = ByteArray::fromBase64(input);
        EXPECT_FALSE(result.hasValue());
    }
}

TEST_F(ByteArrayTest, toBase64)
{
    std::vector<std::pair<ByteArray, std::string>> data{// from RFC 4648:
                                                        {{}, ""},
                                                        {/* "f" */ 0X66_hex, "Zg=="},
                                                        {/* "fo" */ 0X66'6f_hex, "Zm8="},
                                                        {/* "foo" */ 0X66'6f'6f_hex, "Zm9v"},
                                                        {/* "foob" */ 0X66'6f'6f'62_hex,
                                                         "Zm9vYg=="},
                                                        {/* "fooba" */
                                                         0X66'6f'6f'62'61_hex, "Zm9vYmE="},
                                                        {/*"foobar" */ 0X66'6f'6f'62'61'72_hex,
                                                         "Zm9vYmFy"},
                                                        // complete input alphabet:
                                                        {asciiByteArray(), AsciiBase64}};

    for (const auto &[input, expectedOutput] : data) {
        const auto result = input.toBase64();
        EXPECT_EQ(result, expectedOutput);
    }
}

TEST_F(ByteArrayTest, fromBase64_trailingNullBytes)
{
    const std::vector<ByteArray> data{{},
                                      0x00_hex,
                                      0x00'00_hex,
                                      0x00'00'00_hex,
                                      // "f" -> "Zg=="
                                      0x66_hex,
                                      0x66'00_hex,
                                      0x66'00'00_hex,
                                      // "fo" -> "Zm8="
                                      0X66'6f_hex,
                                      0X66'6f'00_hex,
                                      // "foo" -> "Zm9v"
                                      0X66'6f'6f_hex,
                                      0X66'6f'6f'00_hex};

    for (const auto &input : data) {
        const auto base64 = input.toBase64();
        const auto resultArr = ByteArray::fromBase64(base64);
        EXPECT_EQ(*resultArr, input);
    }
}
