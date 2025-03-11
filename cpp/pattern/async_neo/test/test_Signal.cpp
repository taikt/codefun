/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 *     Alexey Timofeyev <alexey.timofeyev@partner.bmw.de>
 */

#include <nf/Context.h>
#include <nf/Signal.h>
#include <nf/testing/Test.h>
#include <nf/testing/TestExecutor.h>
#include <nf/testing/TestTracers.h>

#include <tuple>

using namespace nf;
using namespace nf::testing;

class SignalTest : public nf::testing::Test
{
};

template <typename T>
class TypedSignalTest : public nf::testing::Test
{
};
TYPED_TEST_SUITE_P(TypedSignalTest);

TYPED_TEST_P(TypedSignalTest, directSignalDoesNotCopyArguments)
{
    CopyTracer t("hello");
    std::string receivedValue;

    const Signal<TypeParam> sig;
    auto sub = sig.subscribe([&](const CopyTracer &t) { receivedValue = t->value; });

    sig.notify(t);

    EXPECT_EQ(0, t->cntCopied);
    EXPECT_EQ(0, t->cntMoved);
    EXPECT_EQ("hello", receivedValue);
}

TYPED_TEST_P(TypedSignalTest, indirectSignalCopyArgumentsOnlyOnce)
{
    CopyTracer t("hello");
    std::string receivedValue;

    const Signal<TypeParam> sig;
    auto sub = sig.subscribe(Executor::mainThread(),
                             [&](const CopyTracer &t) { receivedValue = t->value; });

    sig.notify(t);
    EXPECT_EQ("", receivedValue);

    processEvents();

    EXPECT_EQ(1, t->cntCopied);
    EXPECT_EQ(1, t->cntMoved);
    EXPECT_EQ("hello", receivedValue);
}

using CopyTracerTypes = ::testing::Types<CopyTracer, const CopyTracer &>;
REGISTER_TYPED_TEST_SUITE_P(TypedSignalTest, directSignalDoesNotCopyArguments,
                            indirectSignalCopyArgumentsOnlyOnce);
INSTANTIATE_TYPED_TEST_SUITE_P(ValVsConstRef, TypedSignalTest, CopyTracerTypes, );

TEST_F(SignalTest, unsavedSubscriptionMeansNoCall)
{
    int receivedValue = 0;
    Signal<int> sig;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
    sig.subscribe([&](int value) { receivedValue = value; });
#pragma GCC diagnostic pop

    sig.notify(42);

    EXPECT_EQ(0, receivedValue);
}

TEST_F(SignalTest, subscriptionBoolOperator)
{
    const Signal<> sig;

    Subscription sub;
    EXPECT_FALSE(sub);

    sub = sig.subscribe([&] {});
    EXPECT_TRUE(sub);

    sig.notify();
    EXPECT_TRUE(sub);

    sub.reset();
    EXPECT_FALSE(sub);
}

TEST_F(SignalTest, unsubscribeAfterNotify)
{
    int receivedValue = 0;
    const Signal<int> sig;

    auto sub = sig.subscribe(Executor::mainThread(), [&](int value) { receivedValue = value; });
    sig.notify(42);
    sub.reset();

    processEvents();
    EXPECT_EQ(0, receivedValue);
}

TEST_F(SignalTest, unsubscribeSelfDuringNotify)
{
    int receivedValue = 0;
    const Signal<int> sig;

    Subscription sub;
    sub = sig.subscribe([&](int value) {
        receivedValue = value;
        sub.reset();
    });

    sig.notify(2);
    EXPECT_EQ(2, receivedValue);

    // The second notify shall have no effect:
    sig.notify(10);
    EXPECT_EQ(2, receivedValue);
}

TEST_F(SignalTest, unsubscribeOtherDuringNotify)
{
    int receivedValues[] = {0, 0};
    const Signal<int> sig;

    Subscription sub0;
    Subscription sub1;

    sub0 = sig.subscribe([&](int value) {
        receivedValues[0] = value;
        sub1.reset();
    });
    sub1 = sig.subscribe([&](int value) { receivedValues[1] = value; });

    /* The second callback shall never be invoked because the subscription is reset in the
     * first callback. */

    sig.notify(2);
    EXPECT_EQ(2, receivedValues[0]);
    EXPECT_EQ(0, receivedValues[1]);

    sig.notify(3);
    EXPECT_EQ(3, receivedValues[0]);
    EXPECT_EQ(0, receivedValues[1]);
}

TEST_F(SignalTest, subscriptionSet)
{
    int receivedInt{0};
    std::string receivedString;

    const Signal<int> sig1;
    const Signal<std::string> sig2;

    Context context;
    context.bind(sig1.subscribe([&](int i) { receivedInt = i; }));
    context.bind(sig2.subscribe([&](const std::string &s) { receivedString = s; }));

    sig1.notify(5);
    sig2.notify("hello");
    EXPECT_EQ(5, receivedInt);
    EXPECT_EQ("hello", receivedString);

    context.reset();

    sig1.notify(10);
    sig2.notify("world");
    EXPECT_EQ(5, receivedInt);
    EXPECT_EQ("hello", receivedString);
}

TEST_F(SignalTest, subscriptionToContext_resetSubscription)
{
    int receivedInt{0};

    const Signal<int> sig;
    auto sub = sig.subscribe([&receivedInt](int i) { receivedInt = i; });

    Context context;
    context.bind(std::move(sub));

    sig.notify(30);
    EXPECT_EQ(30, receivedInt);

    /* This must not affect this subscription because the ownership is already transferred
     * to the context. */
    sub.reset(); // NOLINT(bugprone-use-after-move,hicpp-invalid-access-moved)

    sig.notify(99);
    EXPECT_EQ(99, receivedInt);

    context.reset();
    sig.notify(1);
    EXPECT_EQ(99, receivedInt);
}

TEST_F(SignalTest, subscriptionToContext_keepSubscription)
{
    int receivedInt{0};

    const Signal<int> sig;
    auto sub = sig.subscribe([&receivedInt](int i) { receivedInt = i; });

    Context context;
    context.bind(std::move(sub));

    sig.notify(99);
    EXPECT_EQ(99, receivedInt);

    context.reset();

    /* Even though 'sub' is still not reset, it does not hold a subscription because it
     * was transferred to the context. The context got reset and the subscription must
     * be now terminated. */
    sig.notify(1);
    EXPECT_EQ(99, receivedInt);
}

TEST_F(SignalTest, implicitArgConversion)
{
    std::string receivedString;

    const Signal<const char *> sig;
    auto sub = sig.subscribe([&](std::string s) { receivedString = std::move(s); });

    sig.notify("chtulhu");
    EXPECT_EQ("chtulhu", receivedString);
}

TEST_F(SignalTest, genericLambda)
{
    std::string receivedString;

    const Signal<const char *> sig;
    auto sub = sig.subscribe([&](auto s) { receivedString = std::move(s); });

    sig.notify("chtulhu");
    EXPECT_EQ("chtulhu", receivedString);
}

TEST_F(SignalTest, genericLambdaWithConstRefereneceParameter)
{
    std::string receivedString;

    const Signal<const char *> sig;
    auto sub = sig.subscribe([&](const auto &s) { receivedString = s; });

    sig.notify("chtulhu");
    EXPECT_EQ("chtulhu", receivedString);
}

TEST_F(SignalTest, variadicGenericLambda)
{
    std::string receivedString;
    int receivedInt{0};

    const Signal<const char *, int> sig;
    auto sub = sig.subscribe([&](auto... pack) {
        std::forward_as_tuple(receivedString, receivedInt) = std::make_tuple(pack...);
    });

    sig.notify("chtulhu", 5);
    EXPECT_EQ("chtulhu", receivedString);
    EXPECT_EQ(5, receivedInt);
}

TEST_F(SignalTest, variadicGenericLambdaOfUniversalReferences)
{
    std::string receivedString;
    int receivedInt{0};

    const Signal<const char *, int> sig;
    auto sub = sig.subscribe([&](auto &&...pack) {
        std::forward_as_tuple(receivedString, receivedInt) = std::forward_as_tuple(pack...);
    });

    sig.notify("chtulhu", 5);
    EXPECT_EQ("chtulhu", receivedString);
    EXPECT_EQ(5, receivedInt);
}

TEST_F(SignalTest, notifyConst)
{
    int receivedValue = 0;

    struct
    {
        void notify(int value) const
        {
            sig.notify(value);
        }

        const Signal<int> sig{};
    } s;

    const Context ctx;
    s.sig.subscribe(ctx, [&](int value) { receivedValue = value; });
    s.notify(42);

    processEvents();
    EXPECT_EQ(42, receivedValue);
}

TEST_F(SignalTest, context_everythingExists)
{
    int receivedValue = 0;
    const Signal<int> sig;

    const Context ctx;
    sig.subscribe(ctx, [&](int value) { receivedValue = value; });
    sig.notify(42);

    processEvents();
    EXPECT_EQ(42, receivedValue);
}

TEST_F(SignalTest, context_deferred)
{
    int receivedValue = 0;
    const Signal<int> sig;

    auto shadowExecutor = std::make_shared<TestExecutor>();
    Context ctx{shadowExecutor};
    sig.subscribe(ctx, [&](int value) { receivedValue = value; });

    sig.notify(42);
    EXPECT_EQ(0, receivedValue);

    processEvents();
    EXPECT_EQ(0, receivedValue);

    shadowExecutor->processEvents();
    EXPECT_EQ(42, receivedValue);
}

TEST_F(SignalTest, context_bindNotDeferred)
{
    int receivedValue = 0;
    const Signal<int> sig;

    auto shadowExecutor = std::make_shared<TestExecutor>();
    const Context ctx{shadowExecutor};
    ctx.bind(sig.subscribe([&](int value) { receivedValue = value; }));
    sig.notify(42);

    EXPECT_EQ(42, receivedValue);
}

TEST_F(SignalTest, context_signalDestroyed)
{
    int receivedValue = 0;
    const Context ctx;

    {
        Signal<int> sig;
        sig.subscribe(ctx, [&](int value) { receivedValue = value; });
        sig.notify(42);
    }

    processEvents();
    EXPECT_EQ(0, receivedValue);
}

TEST_F(SignalTest, context_destroyedBeforeNotify)
{
    int receivedValue = 0;
    const Signal<int> sig;

    {
        const Context ctx;
        sig.subscribe(ctx, [&](int value) { receivedValue = value; });
    }

    sig.notify(42);

    processEvents();
    EXPECT_EQ(0, receivedValue);
}

TEST_F(SignalTest, context_destroyedBeforeEventsProcessed)
{
    int receivedValue = 0;
    const Signal<int> sig;

    {
        const Context ctx;
        sig.subscribe(ctx, [&](int value) { receivedValue = value; });
        sig.notify(42);
    }

    processEvents();
    EXPECT_EQ(0, receivedValue);
}

TEST_F(SignalTest, context_destroyedAfterEventsProcessed)
{
    int receivedValue = 0;
    const Signal<int> sig;

    {
        const Context ctx;
        sig.subscribe(ctx, [&](int value) { receivedValue = value; });
        sig.notify(42);
        processEvents();
    }

    EXPECT_EQ(42, receivedValue);
}

TEST_F(SignalTest, aggregateInitialization)
{
    /* See https://stackoverflow.com/q/56745324/2508253. */
    const Signal<int> sig{};
}
