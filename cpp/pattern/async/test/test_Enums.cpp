/*
 * BMW Neo Framework
 *
 * Copyright (C) 2018 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include <nf/Enums.h>
#include <nf/testing/Test.h>

NF_ENUM(Enum, One, Two, Three)

TEST(EnumsTest, freeEnums)
{
    EXPECT_TRUE(isValid(Enum::One));
    EXPECT_STREQ("Two", toString(Enum::Two));
    EXPECT_EQ(Enum::Three, fromString("Three", Enum::One));
    EXPECT_EQ(Enum::One, fromString("Four", Enum::One));
    EXPECT_EQ("<undefined>", toString(static_cast<Enum>(42)));
}

TEST(EnumsTest, fullConstexprSupport)
{
    constexpr auto *str = toString(Enum::Two);
    EXPECT_EQ("Two", str);

    constexpr bool valid = isValid(Enum::Three);
    EXPECT_TRUE(valid);
}

class MyClass
{
public:
    enum class Enum
    {
        Hello,
        World
    };
};
NF_ENUM_DEFINE_UTILS_C(MyClass, Enum, Hello, World)

TEST(EnumsTest, classEnums)
{
    EXPECT_STREQ("Hello", toString(MyClass::Enum::Hello));
    EXPECT_EQ(MyClass::Enum::World, fromString("World", MyClass::Enum::Hello));
}

class SecondClass
{
public:
    NF_ENUM_CLASS(Letters, A, B, C)
};

TEST(EnumsTest, EnumInClass)
{
    EXPECT_STREQ("A", toString(SecondClass::Letters::A));
    EXPECT_EQ(SecondClass::Letters::B, fromString("B", SecondClass::Letters::C));
    EXPECT_EQ(SecondClass::Letters::C, fromString("!", SecondClass::Letters::C));
}

struct StrangeEnum
{
    enum Literal
    {
        One = 1,
        Two = 3000,
        Three = 9,
        Invalid = 0
    };
};

NF_ENUM_DEFINE_UTILS_C(StrangeEnum, Literal, One, Two, Three, Invalid)

TEST(EnumsTest, NonContinuousEnum)
{
    EXPECT_STREQ("One", toString(StrangeEnum::One));
    EXPECT_STREQ("Two", toString(StrangeEnum::Two));
    EXPECT_STREQ("Three", toString(StrangeEnum::Three));
    EXPECT_EQ(StrangeEnum::One, fromString("One", StrangeEnum::Invalid));
    EXPECT_EQ(StrangeEnum::Two, fromString("Two", StrangeEnum::Invalid));
    EXPECT_EQ(StrangeEnum::Three, fromString("Three", StrangeEnum::Invalid));
}
