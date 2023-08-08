/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019-2020 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Thomas Leichtle <thomas.leichtle@bmw.de>
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include <nf/Assert.h>
#include <nf/testing/Test.h>

#include <optional>

using namespace testing;
using namespace nf::testing;

class AssertTest : public nf::testing::Test
{
};

TEST_F(AssertTest, expectWithReturnValue)
{
    // clang-format off
    auto testFun = [](bool isSuccess, bool resultOnFailure) -> std::optional<bool> {
        if(!nf::expectThat(isSuccess, "errorcode {}: {:d}", 42, resultOnFailure))
        {
            return resultOnFailure;
        }
        return std::nullopt;
    };

    /* Do not intervene if condition is fulfilled. */
    const size_t numOfLogMsgs = logMessages().size();
    EXPECT_EQ(std::nullopt, testFun(true, false));
    EXPECT_EQ(numOfLogMsgs, logMessages().size());

    /* Return the correct value when condition is not fulfilled and perform logging. */
    EXPECT_EQ(true, testFun(false, true));
    EXPECT_THAT(logMessages().back(), EndsWith("errorcode 42: 1"));

    EXPECT_EQ(false, testFun(false, false));
    EXPECT_THAT(logMessages().back(), EndsWith("errorcode 42: 0"));
}

TEST_F(AssertTest, expectWithoutReturnValue)
{
    int cntInvoked = 0;

    auto testFun = [&](bool isSuccess) {
        if(!nf::expectThat(isSuccess, "Error")){
            return;
        }
        cntInvoked++;
    };

    /* Function is executed till the end and no error log is printed. */
    const size_t numOfLogMsgs = logMessages().size();
    testFun(true);
    EXPECT_EQ(numOfLogMsgs, logMessages().size());
    EXPECT_EQ(1, cntInvoked);

    /* Function returns early and an error log is printed. */
    testFun(false);
    EXPECT_THAT(logMessages().back(), EndsWith("Error"));
    EXPECT_EQ(1, cntInvoked);
}

TEST_F(AssertTest, assertAndReturnConstRef)
{
    auto testFun = [](bool isSuccess, const int &returnValue) -> const int & {
        nf::assertThat(isSuccess, "Not a success");
        return returnValue;
    };

    int val = 2020;

    /* No crash and no logs in case of a success. */
    const size_t numOfLogMsgs = logMessages().size();
    EXPECT_EQ(2020, testFun(true, val));
    EXPECT_EQ(numOfLogMsgs, logMessages().size());

    /* There must be a crash instead of a failure. */
    auto noCoreDumps = disableCoreDumps();
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    ASSERT_DEATH({ testFun(false, val); }, "");
}
