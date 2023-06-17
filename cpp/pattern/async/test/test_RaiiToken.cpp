/*
 * BMW Neo Framework
 *
 * Copyright (C) 2021-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 *     Szymon Wojtczak <szymon.wojtczak@globallogic.com>
 */

#include <nf/RaiiToken.h>
#include <nf/testing/Test.h>

using namespace nf;
using namespace nf::testing;

TEST(RaiiTokenTest, canBeDefaultConstructed)
{
    RaiiToken token;
}

TEST(RaiiTokenTest, canBeMoveConstructedDismissible)
{
    int cntInvoked = 0;

    {
        auto token = RaiiToken::dismissible([&] { cntInvoked += 1; });
        RaiiToken moved = std::move(token);
        EXPECT_EQ(0, cntInvoked);
    }

    EXPECT_EQ(1, cntInvoked);
}

TEST(RaiiTokenTest, canBeMoveConstructedNonDismissible)
{
    int cntInvoked = 0;

    {
        auto token = RaiiToken::nonDismissible([&] { cntInvoked += 1; });
        RaiiToken moved = std::move(token);
        EXPECT_EQ(0, cntInvoked);
    }

    EXPECT_EQ(1, cntInvoked);
}

TEST(RaiiTokenTest, canBeMoveAssigned)
{
    int cntInvoked = 0;

    {
        auto tokenA = RaiiToken::nonDismissible([&] { cntInvoked += 1; });
        auto tokenB = RaiiToken::nonDismissible([&] { cntInvoked += 10; });
        EXPECT_EQ(0, cntInvoked);

        tokenB = std::move(tokenA);
        EXPECT_EQ(10, cntInvoked);
    }

    EXPECT_EQ(11, cntInvoked);
}

TEST(RaiiTokenTest, canBeMoveAssignedToDefaultConstructed)
{
    int cntInvoked = 0;

    {
        RaiiToken tokenA;
        auto tokenB = RaiiToken::nonDismissible([&] { cntInvoked += 10; });
        EXPECT_EQ(0, cntInvoked);

        tokenA = std::move(tokenB);
        EXPECT_EQ(0, cntInvoked);
    }

    EXPECT_EQ(10, cntInvoked);
}

TEST(RaiiTokenTest, resetWhenDefaultConstructed)
{
    RaiiToken token;
    try {
        token.reset();
        SUCCEED();
    } catch (...) {
        FAIL() << "Should never throw.";
    }
}

TEST(RaiiTokenTest, canBeReturned)
{
    bool isInvoked = false;

    auto acquire = [&] { return RaiiToken::nonDismissible([&] { isInvoked = true; }); };

    {
        auto token = acquire();
        EXPECT_FALSE(isInvoked);
    }

    EXPECT_TRUE(isInvoked);
}

TEST(RaiiTokenTest, reset)
{
    int cntInvoked = 0;

    {
        auto token = RaiiToken::nonDismissible([&] { cntInvoked += 1; });
        EXPECT_EQ(0, cntInvoked);
        token.reset();
        EXPECT_EQ(1, cntInvoked);
    }

    EXPECT_EQ(1, cntInvoked);
}

TEST(RaiiTokenTest, resetTwice)
{
    int cntInvoked = 0;

    {
        auto token = RaiiToken::nonDismissible([&] { cntInvoked += 1; });
        EXPECT_EQ(0, cntInvoked);
        token.reset();
        EXPECT_EQ(1, cntInvoked);
        token.reset();
        EXPECT_EQ(1, cntInvoked);
    }

    EXPECT_EQ(1, cntInvoked);
}

TEST(RaiiTokenTest, dismissDismissible)
{
    int cntInvoked = 0;

    {
        auto token = RaiiToken::dismissible([&] { cntInvoked += 1; });
        EXPECT_EQ(0, cntInvoked);
        token.dismiss();
        EXPECT_EQ(0, cntInvoked);
    }

    EXPECT_EQ(0, cntInvoked);
}

TEST(RaiiTokenTest, dismissNonDismissible)
{
    int cntInvoked = 0;
    {
        auto token = RaiiToken::nonDismissible([&] { cntInvoked += 1; });
        EXPECT_EQ(0, cntInvoked);
        try {
            token.dismiss();
            FAIL() << "std::runtime_error should have been thrown.";
        } catch (const std::runtime_error &error) {
            EXPECT_EQ(0, cntInvoked);
        } catch (...) {
            FAIL() << "std::runtime_error should have been thrown.";
        }
    }
}

TEST(RaiiTokenTest, dismissAndReset)
{
    int cntInvoked = 0;

    {
        auto token = RaiiToken::dismissible([&] { cntInvoked += 1; });
        EXPECT_EQ(0, cntInvoked);
        token.dismiss();
        EXPECT_EQ(0, cntInvoked);
        token.reset();
        EXPECT_EQ(0, cntInvoked);
    }

    EXPECT_EQ(0, cntInvoked);
}

TEST(RaiiTokenTest, exceptionThrown)
{
    auto noCoreDumps = disableCoreDumps();
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    ASSERT_DEATH({ auto token = RaiiToken::nonDismissible([] { throw std::exception(); }); }, "");
}
