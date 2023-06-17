/*
 * BMW Neo Framework
 *
 * Copyright (C) 2020-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 *     Thomas Leichtle <thomas.leichtle@bmw.de>
 */

#include <nf/Attribute.h>
#include <nf/Context.h>
#include <nf/testing/Test.h>

using ::testing::_; // NOLINT

namespace {

using AttributeTest = nf::testing::Test;

struct SomeStruct
{
    int a{0};
    int b{0};
    bool operator==(const SomeStruct &rhs) const noexcept
    {
        return rhs.a == a && rhs.b == b;
    }
    bool operator!=(const SomeStruct &rhs) const noexcept
    {
        return !(*this == rhs);
    }
};

struct Object
{
    MOCK_METHOD(void, signalInt, (int));
    MOCK_METHOD(void, signalDouble, (double));
    MOCK_METHOD(void, signalStruct, (SomeStruct));
    MOCK_METHOD(void, signalIntWithDefault, (int));

    nf::Attribute<int> intAttr;
    nf::Attribute<double> doubleAttr;
    nf::Attribute<SomeStruct> structAttr;
    nf::Attribute<int> intWithDefault{2};

    Object() noexcept
    {
        m_context.bind(intAttr.changed.subscribe([this](int i) { signalInt(i); }));
        m_context.bind(doubleAttr.changed.subscribe([this](double d) { signalDouble(d); }));
        m_context.bind(
            structAttr.changed.subscribe([this](const SomeStruct &s) { signalStruct(s); }));
        m_context.bind(
            intWithDefault.changed.subscribe([this](int i) { signalIntWithDefault(i); }));
    }

private:
    nf::Context m_context;
};

} // anonymous namespace

/// Attributes can hold integer.
TEST_F(AttributeTest, withInteger)
{
    Object obj;

    ::testing::InSequence seq;
    EXPECT_CALL(obj, signalInt(42)).Times(1);
    EXPECT_CALL(obj, signalInt(22)).Times(1);

    obj.intAttr.set(42);
    obj.intAttr.set(42);
    obj.intAttr.set(22);
}

/// Attributes can hold double.
TEST_F(AttributeTest, withDouble)
{
    Object obj;

    EXPECT_CALL(obj, signalDouble(_)).Times(2);

    obj.doubleAttr.set(5.5);
    obj.doubleAttr.set(5.5);
    obj.doubleAttr.set(10.2);
    obj.doubleAttr.set(10.2);
}

/// Attributes can hold structs.
TEST_F(AttributeTest, withStruct)
{
    Object obj;

    // Initial values will not trigger signal.
    EXPECT_CALL(obj, signalStruct(_)).Times(0);
    obj.structAttr.set({0, 0});

    // Changing values will trigger the signal once.
    EXPECT_CALL(obj, signalStruct(_)).Times(1);
    obj.structAttr.set({1, 2});
    obj.structAttr.set({1, 2});
}

/// Attributes can be default initialized.
TEST_F(AttributeTest, withDefault)
{
    Object obj;

    // Initial value is assigned.
    ASSERT_EQ(2, obj.intWithDefault.value());

    // Initial values will not trigger signal.
    EXPECT_CALL(obj, signalIntWithDefault(_)).Times(0);
    obj.intWithDefault.set(2);

    // Changing values will trigger the signal once.
    EXPECT_CALL(obj, signalIntWithDefault(3)).Times(1);
    obj.intWithDefault.set(3);
}
