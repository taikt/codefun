/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019-2021 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include <nf/Function.h>
#include <nf/testing/Test.h>
#include <nf/testing/TestTracers.h>

using namespace nf;
using namespace nf::testing;

class FunctionTest : public nf::testing::Test
{
};

TEST_F(FunctionTest, argByValue_copy)
{
    // NOLINTNEXTLINE Pass by value is on purpose.
    Function<int(CopyTracer)> f = [](CopyTracer ct) { return 2 + std::stoi(ct->value); };

    CopyTracer ct("30");
    int value = f(ct);

    EXPECT_EQ(1, ct->cntCopied);
    EXPECT_GE(1, ct->cntMoved);
    EXPECT_EQ(32, value);
}

TEST_F(FunctionTest, argByValue_const)
{
    // NOLINTNEXTLINE Pass by value is on purpose.
    Function<int(CopyTracer)> f = [](CopyTracer ct) { return 2 + std::stoi(ct->value); };

    const CopyTracer ct("30");
    int value = f(ct);

    EXPECT_EQ(1, ct->cntCopied);
    EXPECT_GE(1, ct->cntMoved);
    EXPECT_EQ(32, value);
}

TEST_F(FunctionTest, argByValue_move)
{
    // NOLINTNEXTLINE Pass by value is on purpose.
    Function<int(CopyTracer)> f = [](CopyTracer ct) { return 2 + std::stoi(ct->value); };

    CopyTracer ct("40");
    auto sb = ct.sb();
    int value = f(std::move(ct));

    EXPECT_EQ(0, sb->cntCopied);
    EXPECT_GE(2, sb->cntMoved);
    EXPECT_EQ(42, value);
}

TEST_F(FunctionTest, argConstRef)
{
    Function<int(const CopyTracer &)> f = [](const CopyTracer &ct) {
        return std::stoi(ct->value) - 10;
    };

    CopyTracer ct("90");
    int value = f(ct);

    EXPECT_EQ(0, ct->cntCopied);
    EXPECT_EQ(0, ct->cntMoved);
    EXPECT_EQ(80, value);
}

TEST_F(FunctionTest, argRef)
{
    Function<int(CopyTracer &)> f = [](CopyTracer &ct) { return std::stoi(ct->value) - 10; };

    CopyTracer ct("99");
    int value = f(ct);

    EXPECT_EQ(0, ct->cntCopied);
    EXPECT_EQ(0, ct->cntMoved);
    EXPECT_EQ(89, value);
}

TEST_F(FunctionTest, argFwdRef)
{
    Function<int(CopyTracer &&)> f = [](CopyTracer &&ct) { return 2 + std::stoi(ct->value); };

    CopyTracer ct("40");
    auto sb = ct.sb();
    int value = f(std::move(ct));

    EXPECT_EQ(0, sb->cntCopied);
    EXPECT_GE(1, sb->cntMoved);
    EXPECT_EQ(42, value);
}

TEST_F(FunctionTest, nonCopyableLambda)
{
    MoveTracer mt("21");
    auto sb = mt.sb();
    int receivedValue = 0;

    Function<void(int)> f = [&, mt = std::move(mt)](int value) {
        receivedValue = std::stoi(mt->value) * value;
    };

    f(2);
    EXPECT_GE(2, sb->cntMoved);
    EXPECT_EQ(42, receivedValue);

    EXPECT_TRUE(f);
    auto movedF = std::move(f);
    EXPECT_FALSE(f); // NOLINT it is okay to check that "moved-out" function is "false"

    movedF(10);
    EXPECT_EQ(0, sb->cntCopied);
    EXPECT_GE(2, sb->cntMoved);
    EXPECT_EQ(210, receivedValue);
}

TEST_F(FunctionTest, assignment)
{
    MoveTracer mt("33");
    auto sb = mt.sb();

    Function<int(int)> f1 = [mt = std::move(mt)](int value) {
        return std::stoi(mt->value) * value;
    };
    Function<int(int)> f2;

    EXPECT_EQ(66, f1(2));

    EXPECT_FALSE(static_cast<bool>(f2));
    f2 = std::move(f1);
    ASSERT_TRUE(static_cast<bool>(f2));
    EXPECT_EQ(99, f2(3));
}

TEST_F(FunctionTest, typeTraits)
{
    EXPECT_TRUE(IsFunction<Function<void()>>);
    EXPECT_TRUE(IsFunction<const Function<void()>>);
    EXPECT_TRUE(IsFunction<Function<void()> &>);
    EXPECT_TRUE(IsFunction<const Function<void()> &>);
    EXPECT_TRUE(IsFunction<Function<void()> &&>);
    EXPECT_FALSE(IsFunction<int>);
}

TEST_F(FunctionTest, typeTraits_noexcept)
{
    EXPECT_TRUE(IsFunction<Function<void() noexcept>>);
    EXPECT_TRUE(IsFunction<const Function<void() noexcept>>);
    EXPECT_TRUE(IsFunction<Function<void() noexcept> &>);
    EXPECT_TRUE(IsFunction<const Function<void() noexcept> &>);
    EXPECT_TRUE(IsFunction<Function<void() noexcept> &&>);
}

TEST_F(FunctionTest, callInConstContext)
{
    auto f1 = nf::Function<int()>([]() { return 42; });

    auto constLambda = [f = std::move(f1)]() { return f(); };
    auto v1 = constLambda();
    EXPECT_EQ(42, v1);

    auto f2 = nf::Function<int()>([]() { return 42; });
    auto mutableLambda = [f = std::move(f2)]() mutable { return f(); };
    auto v2 = mutableLambda();
    EXPECT_EQ(42, v2);
}

TEST_F(FunctionTest, swallowReturnType)
{
    bool called = false;

    auto f = nf::Function<void()>([&]() {
        called = true;
        return true;
    });

    f();

    EXPECT_TRUE(called);
}

TEST_F(FunctionTest, terminateOnEmptyTarget)
{
    Function<void() noexcept> f;

    auto noCoreDumps = nf::testing::disableCoreDumps();
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    ASSERT_DEATH({ f(); }, "");
}

TEST_F(FunctionTest, throwOnEmptyTarget)
{
    Function<void()> f;
    bool isCaught = false;

    try {
        f();
    } catch (const std::bad_function_call &) {
        isCaught = true;
    }

    EXPECT_TRUE(isCaught);
}

TEST_F(FunctionTest, throwFromFunction)
{
    Function<void()> f = [] { throw 5; }; // NOLINT
    int thrownI = 0;

    try {
        f();
    } catch (int i) {
        thrownI = i;
    }

    EXPECT_EQ(5, thrownI);
}
