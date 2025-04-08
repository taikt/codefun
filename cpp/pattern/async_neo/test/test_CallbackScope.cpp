/*
 * BMW Neo Framework
 *
 * Copyright (C) 2018-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include <nf/detail/CallbackScope.h>
#include <nf/testing/Test.h>

#include <functional>

TEST(CallbackScopeTest, callback_isInvoked)
{
    nf::detail::CallbackScope scope;
    auto unique = std::make_unique<int>(42);

    int invokedValue = 0;
    // scoped callbacks must support non-copyable lambdas
    auto cb = scope.bind(
        [&invokedValue, unique = std::move(unique)](int mul) { invokedValue = *unique * mul; });

    cb(2);
    ASSERT_EQ(84, invokedValue);
}

TEST(CallbackScopeTest, callback_outOfScope)
{
    std::function<void()> cb;
    bool isInvoked = false;

    {
        nf::detail::CallbackScope scope;
        cb = scope.bind([&]() { isInvoked = true; });
    }

    cb();
    ASSERT_FALSE(isInvoked);
}

TEST(CallbackScopeTest, callback_withReturnValue_ok)
{
    nf::detail::CallbackScope scope;
    auto cb = scope.bind([](int mul) { return mul * 2; });

    int res = cb(21);
    ASSERT_EQ(42, res);
}

TEST(CallbackScopeTest, callback_withReturnValue_default)
{
    std::function<int(int)> cb;

    {
        nf::detail::CallbackScope scope;
        cb = scope.bind([](int mul) { return mul * 2; });
    }

    int res = cb(21);
    ASSERT_EQ(0, res);
}

TEST(CallbackScopeTest, callback_reset)
{
    std::function<void()> cb;
    bool isInvoked = false;

    nf::detail::CallbackScope scope;
    cb = scope.bind([&]() { isInvoked = true; });

    scope.reset();

    cb();
    ASSERT_FALSE(isInvoked);
}

TEST(CallbackScopeTest, callback_resetInside)
{
    int cntInvocations = 0;

    nf::detail::CallbackScope scope;
    auto cb = scope.bind([&]() {
        scope.reset();
        cntInvocations++;
    });

    cb();
    cb();
    ASSERT_EQ(1, cntInvocations);
}

TEST(CallbackScopeTest, callback_moveTheScope)
{
    std::function<void()> cb;
    bool isInvoked = false;

    auto bindLambda = [&]() {
        nf::detail::CallbackScope scope;
        cb = scope.bind([&]() { isInvoked = true; });
        return scope;
    };

    auto scope = bindLambda();

    cb();
    ASSERT_TRUE(isInvoked);
}
