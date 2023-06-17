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

#include <nf/Context.h>
#include <nf/Executor.h>
#include <nf/RaiiToken.h>
#include <nf/testing/Test.h>
#include <nf/testing/TestExecutor.h>
#include <nf/testing/TestTracers.h>

#include <chrono>

using namespace nf;
using namespace nf::testing;
using namespace std::chrono_literals;

class ContextTest : public Test
{
};

TEST_F(ContextTest, post_defaultExecutor)
{
    bool isInvoked{false};

    Context ctx;
    ctx.post([&] { isInvoked = true; });

    EXPECT_FALSE(isInvoked);
    processEvents();
    EXPECT_TRUE(isInvoked);
}

TEST_F(ContextTest, post_defaultExecutor_delay)
{
    bool isInvoked{false};

    Context ctx;
    ctx.post(100ms, [&] { isInvoked = true; });

    EXPECT_FALSE(isInvoked);
    processEvents();
    EXPECT_FALSE(isInvoked);
    processEvents(100ms);
    EXPECT_TRUE(isInvoked);
}


TEST_F(ContextTest, post_customExecutor)
{
    bool isInvoked{false};

    Context ctx(Executor::mainThread());
    ctx.post([&] { isInvoked = true; });

    EXPECT_FALSE(isInvoked);
    processEvents();
    EXPECT_TRUE(isInvoked);
}

/* Tasks posted to Context shall NOT be executed if Context is destroyed. */
TEST_F(ContextTest, post_contextDestroyed)
{
    bool isInvoked{false};

    bool isTokenDestroyed{false};
    auto token = RaiiToken::nonDismissible([&] { isTokenDestroyed = true; });

    {
        Context ctx;
        ctx.post([&, token = std::move(token)] { isInvoked = true; });
    }

    EXPECT_TRUE(isTokenDestroyed);

    processEvents();
    EXPECT_FALSE(isInvoked);
}

TEST_F(ContextTest, bindLambda_ok)
{
    auto ctx = std::make_shared<Context>();
    auto f = ctx->bind([](int i) { return i * 5; });

    int result{0};
    f(10).then(*ctx, [&](std::optional<int> val) { result = *val; });
    processEvents();
    EXPECT_EQ(50, result);

    result = 0;
    f(10).then(*ctx, [&](std::optional<int> val) { result = *val; });
    processEvents();
    EXPECT_EQ(50, result);
}

TEST_F(ContextTest, bindLambda_nested_ok)
{
    auto ctx = std::make_shared<Context>();
    {
        auto f1 = ctx->bind([](int i) {
            Promise<int> promise;
            promise.fulfill(i * 5);
            return promise.future();
        });

        auto f2 = ctx->bind([ctx, f1 = std::move(f1)](int i) {
            int res{0};
            f1(i).then(*ctx, [&](std::optional<int> val) {
                if (val) {
                    res = *val;
                }
            });
            processEvents();
            return res + 10;
        });

        int res{0};
        f2(10).then(*ctx, [&](std::optional<int> val) {
            if (val) {
                res = *val;
            }
        });

        processEvents();
        EXPECT_EQ(60, res);
    }
}

/* Default-constructed value shall be returned if the context is destroyed. */
TEST_F(ContextTest, bindLambda_contextDestroyed)
{
    bool isTokenDestroyed{false};
    auto token = RaiiToken::nonDismissible([&] { isTokenDestroyed = true; });

    Context ctx;
    auto f = ctx.bind([token = std::move(token)](int i) { return i * 5; });
    ctx.reset();

    std::optional<int> result{0};
    f(10).then(ctx, [&](auto val) { result = val; });
    processEvents();

    EXPECT_TRUE(isTokenDestroyed);
    EXPECT_EQ(std::nullopt, result);
}

/* All service info shall be deleted from the context if lambda is destroyed. */
TEST_F(ContextTest, bindLambda_lambdaDestroyed)
{
    bool isTokenDestroyed{false};
    auto token = RaiiToken::nonDismissible([&] { isTokenDestroyed = true; });

    Context ctx;

    {
        auto f = ctx.bind([token = std::move(token)](int i) { return i * 5; });
    }

    EXPECT_TRUE(isTokenDestroyed);
    ASSERT_TRUE(ctx.isEmpty());
}

TEST_F(ContextTest, deferLambda_ok)
{
    int result{0};

    auto ctx = std::make_shared<Context>();
    // move-only lambda can be called multiple times
    auto f = ctx->bindDeferred([&, r = std::make_unique<int>(5)](int i) { result = i + *r; });

    f(42);
    EXPECT_EQ(0, result);

    processEvents();
    EXPECT_EQ(47, result);

    f(43);
    processEvents();
    EXPECT_EQ(48, result);
}

TEST_F(ContextTest, deferLambda_lvalue_ok)
{
    int result{0};

    auto ctx = std::make_shared<Context>();
    auto f = [&]() {
        auto lambda = [&](int i) { result = i; };
        auto sp = std::make_unique<decltype(lambda)>(std::move(lambda));
        return ctx->bindDeferred(*sp);
    }();

    f(42);
    EXPECT_EQ(0, result);

    processEvents();
    EXPECT_EQ(42, result);

    f(43);
    processEvents();
    EXPECT_EQ(43, result);
}

TEST_F(ContextTest, deferLambda_lvalueArg_ok)
{
    int result{-1};

    auto ctx = std::make_shared<Context>();
    auto f = ctx->bindDeferred([&](const CopyTracer &ct) { result = std::stoi(ct->value); });

    {
        const auto arg = CopyTracer("42");
        f(arg);
        EXPECT_EQ(1, arg->cntCopied);
    }
    EXPECT_EQ(-1, result);

    processEvents();
    EXPECT_EQ(42, result);
}

TEST_F(ContextTest, deferLambda_nonMovableArg_ok)
{
    int result{-1};

    struct CopyOnlyArg
    {
        int mI{31};

        CopyOnlyArg() = default;
        CopyOnlyArg(const CopyOnlyArg &) = default;
        CopyOnlyArg(CopyOnlyArg &&) = delete;
        CopyOnlyArg &operator=(const CopyOnlyArg &) = default;
        CopyOnlyArg &operator=(CopyOnlyArg &&) = delete;
    };

    auto ctx = std::make_shared<Context>();
    auto f = ctx->bindDeferred([&](const CopyOnlyArg &a) { result = a.mI; });

    f(CopyOnlyArg{});
    EXPECT_EQ(-1, result);

    processEvents();
    EXPECT_EQ(31, result);
}

TEST_F(ContextTest, deferLambda_contextDestroyed_beforeCall)
{
    int result{0};

    auto ctx = std::make_shared<Context>();
    auto f = ctx->bindDeferred([&](int i) { result = i; });

    ctx.reset();

    f(42);
    EXPECT_EQ(0, result);

    processEvents();
    EXPECT_EQ(0, result);
}

TEST_F(ContextTest, deferLambda_contextDestroyed_afterCall)
{
    int result{0};

    auto ctx = std::make_shared<Context>();
    auto f = ctx->bindDeferred([&result, r = std::make_unique<int>(5)](MoveTracer mt) {
        result = std::stoi(mt->value) + *r;
    });

    auto arg = MoveTracer("42");
    f(std::move(arg));

    EXPECT_EQ(0, result);

    ctx.reset();

    processEvents();
    EXPECT_EQ(0, result);
}

TEST_F(ContextTest, deferLambda_lambdaDestroyed_resources)
{
    bool isTokenDestroyed{false};
    auto token = RaiiToken::nonDismissible([&] { isTokenDestroyed = true; });

    Context ctx;

    {
        auto f = ctx.bindDeferred([token = std::move(token)](int i) { return i * 5; });
    }

    EXPECT_TRUE(isTokenDestroyed);
    ASSERT_TRUE(ctx.isEmpty());
}

TEST_F(ContextTest, deferLambda_contextDestroyed_resources)
{
    bool isTokenDestroyed{false};
    auto token = RaiiToken::nonDismissible([&] { isTokenDestroyed = true; });

    Context ctx;
    auto f = ctx.bindDeferred([token = std::move(token)](int i) { return i * 5; });

    ctx.reset();

    EXPECT_TRUE(isTokenDestroyed);
    ASSERT_TRUE(ctx.isEmpty());
}

TEST_F(ContextTest, deferLambda_executor)
{
    int result{0};

    auto shadowExecutor = std::make_shared<TestExecutor>();
    auto ctx = Context{shadowExecutor};
    auto f = ctx.bindDeferred([&](int i) { result = i; });

    f(42);
    EXPECT_EQ(0, result);

    processEvents();
    EXPECT_EQ(0, result);

    shadowExecutor->processEvents();
    EXPECT_EQ(42, result);
}

TEST_F(ContextTest, bindLambda_defer_void_resulting)
{
    int result{0};

    auto shadowExecutor = std::make_shared<TestExecutor>();
    auto ctx = Context{shadowExecutor};

    std::thread thread([&result, &ctx]() {
        auto f = ctx.bind([&](int i) { result = i; });
        f(42);
        processEvents();
    });
    thread.join();

    EXPECT_EQ(0, result);

    processEvents();
    EXPECT_EQ(0, result);

    shadowExecutor->processEvents();
    EXPECT_EQ(42, result);
}

TEST_F(ContextTest, bindLambda_defer_non_void_resulting)
{
    int result{0};

    auto shadowExecutor = std::make_shared<TestExecutor>();
    auto ctx = Context{shadowExecutor};

    std::thread thread([&result, &ctx]() {
        auto f = ctx.bind([&](int i) { return i; });
        f(42).then(ctx, [&](std::optional<int> i) { result = *i; });
        processEvents();
    });
    thread.join();

    EXPECT_EQ(0, result);

    processEvents();
    EXPECT_EQ(0, result);

    shadowExecutor->processEvents();
    EXPECT_EQ(42, result);
}

TEST_F(ContextTest, bindLambda_invoke_from_other_thread)
{
    int result{0};

    auto ctx = std::make_shared<Context>();

    std::thread thread([&result, &ctx]() {
        auto f = ctx->bind([&](int i) {
            Promise<int> promise;
            promise.fulfill(i);
            return promise.future();
        });

        f(42).then(*ctx, [&](std::optional<int> i) { result = *i; });
    });
    thread.join();
    EXPECT_EQ(0, result);

    processEvents();
    EXPECT_EQ(42, result);
}

TEST_F(ContextTest, isThisThread)
{
    Context ctx;
    ASSERT_TRUE(ctx.isThisThread());
}

TEST_F(ContextTest, isOtherThread)
{
    auto executor = std::make_shared<TestExecutor>();
    Context ctx(executor);
    ASSERT_FALSE(ctx.isThisThread());
}

TEST_F(ContextTest, genericLambda)
{
    int result{0};

    Context ctx;
    auto f = ctx.bind([&](auto i) { result = i; });

    f(42);
    EXPECT_EQ(42, result);
}

TEST_F(ContextTest, genericLambda_moveOnlyArg)
{
    Context ctx;
    auto f = ctx.bind([&](auto i) { return *i + 3; });

    std::optional<int> result{0};
    f(std::make_unique<int>(2)).then(ctx, [&result](std::optional<int> val) { result = val; });
    processEvents();

    EXPECT_EQ(5, *result);
}

TEST_F(ContextTest, genericLambda_deferred)
{
    int result{0};

    Context ctx;
    auto f = ctx.bindDeferred([&](auto i) { result = i; });

    f(42);
    EXPECT_EQ(0, result);

    processEvents();
    EXPECT_EQ(42, result);
}

TEST_F(ContextTest, genericLambda_resourcesReleased)
{
    bool isTokenDestroyed{false};
    auto token = RaiiToken::nonDismissible([&] { isTokenDestroyed = true; });

    Context ctx;
    auto f = ctx.bind([token = std::move(token)](int i) { return i * 5; });

    std::optional<int> result{0};
    f(10).then(ctx, [&](std::optional<int> val) { result = val; });
    processEvents();

    EXPECT_FALSE(isTokenDestroyed);
    EXPECT_EQ(50, *result);

    ctx.reset();
    f(20).then(ctx, [&](std::optional<int> val) { result = val; });
    processEvents();

    EXPECT_TRUE(isTokenDestroyed);
    EXPECT_EQ(std::nullopt, result);
}

TEST_F(ContextTest, exceptionThrown)
{
    Context ctx;
    auto f = ctx.bind([] { throw std::runtime_error("error"); });
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    EXPECT_THROW(f(), std::runtime_error);
}

TEST_F(ContextTest, bindRaiiToken)
{
    bool isTokenDestroyed{false};
    auto token = RaiiToken::nonDismissible([&] { isTokenDestroyed = true; });

    Context ctx;
    ctx.bind(std::move(token));
    EXPECT_FALSE(isTokenDestroyed);

    ctx.reset();
    EXPECT_TRUE(isTokenDestroyed);
}

TEST_F(ContextTest, bindRaiiToken_outOfScope)
{
    bool isTokenDestroyed{false};
    Context ctx;

    {
        auto token = RaiiToken::nonDismissible([&] { isTokenDestroyed = true; });
        ctx.bind(std::move(token));
    }
    EXPECT_FALSE(isTokenDestroyed);

    ctx.reset();
    EXPECT_TRUE(isTokenDestroyed);
}
