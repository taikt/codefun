/*
 * BMW Neo Framework
 *
 * Copyright (C) 2021-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 *     Manuel Nickschas <manuel.nickschas@bmw.de>
 */

#include <nf/Async.h>
#include <nf/Composition.h>
#include <nf/Context.h>
#include <nf/Executor.h>
#include <nf/Future.h>
#include <nf/Result.h>
#include <nf/async/Execute.h>
#include <nf/testing/Test.h>
#include <nf/testing/TestTracers.h>

#include <boost/hana/functional/overload.hpp>

#include <condition_variable>
#include <optional>
#include <string>
#include <thread>
#include <type_traits>

using namespace nf;
using namespace nf::async;
using namespace nf::testing;

using namespace std::chrono_literals;

using ::testing::NotNull;

template <typename T>
void callPromise(const std::function<void(T)> &cb, T val)
{
    cb(val);
}

void callPromise(const std::function<void()> &cb)
{
    cb();
}

int twice(int i)
{
    return i + i;
}

float toFloat(const std::string &str)
{
    return std::stof(str);
}

class FuturePromise : public Test
{
};

TEST_F(FuturePromise, verifyCompilation)
{
    using IntPtr = std::unique_ptr<int>;
    nf::Context scope;

    // T0 -> T1
    {
        auto p = Promise<std::unique_ptr<int>>{};
        p.future().then(scope,
                        [b = IntPtr{}](std::unique_ptr<int> /*unused*/) -> std::unique_ptr<int> {
                            return {};
                        });
    }
    // T0 -> void
    {
        auto p = Promise<std::unique_ptr<int>>{};
        p.future().then(scope, [b = IntPtr{}](std::unique_ptr<int> /*unused*/) -> void {});
    }
    // void -> T1
    {
        auto p = Promise<void>{};
        p.future().then(scope, [b = IntPtr{}]() -> std::unique_ptr<int> { return {}; });
    }
    // void -> void
    {
        auto p = Promise<void>{};
        p.future().then(scope, [b = IntPtr{}]() -> void {});
    }
    // void -> void -> void
    {
        Future<void> future;
        future                                          //
            .then(scope, [] { return Future<void>(); }) //
            .then(scope, []() {});
    }
    // Nested future
    // {
    // T0 -> future -> T1
    {
        auto p = Promise<int>{};
        p.future()
            .then(scope, [&, b = IntPtr{}](int i) { return Future<int>{i}; })
            .then(scope, [&, b = IntPtr{}](int i) -> std::string { return std::to_string(i); });
    }
    // void -> future -> void
    {
        auto p = Promise<void>{};
        p.future()
            .then(scope, [&, b = IntPtr{}]() { return Future<int>{1}; })
            .then(scope, [&, b = IntPtr{}](int /*unused*/) {});
    }
    // execute
    {
        execute(
            scope, [b = IntPtr{}](std::unique_ptr<int> /*unused*/) -> int { return {}; },
            std::make_unique<int>(0));
        execute(
            scope, [b = IntPtr{}](std::unique_ptr<int> /*unused*/) {}, std::make_unique<int>(0));
        execute(scope, [b = IntPtr{}]() -> std::unique_ptr<int> { return {}; });
        execute(scope, [b = IntPtr{}]() {});
    }

    // Ready future
    {
        struct Structure
        {
            // NOLINTNEXTLINE(google-explicit-constructor)
            Structure(int i)
                : mStr{std::to_string(i)}
            {
            }
            std::string mStr;
        };

        [[maybe_unused]] auto a = Future<Structure>{1};

        [&]() -> Future<int> { return Future{1}; }();

        [&]() -> Future<void> { return {}; }();

        [&]() {
            auto str = std::string{"boo"};
            auto f = [](const std::string &s) { return Future{s}; }(str);
            return f;
        }();
    }
}

TEST_F(FuturePromise, simpleContinuation)
{
    nf::Context context;

    // With void
    {
        bool finished = false;
        auto promise = Promise<void>{};
        promise.future().then(context, [&]() { finished = true; });

        promise.fulfill();
        processEvents();

        ASSERT_TRUE(finished);
    }
    // Copyable value
    {
        std::string result;

        auto promise = Promise<std::string>{};
        promise.future().then(context, [&](std::string val) { result = std::move(val); });

        promise.fulfill("gotcha!");
        processEvents();

        ASSERT_TRUE(result == "gotcha!");
    }
    // Move-only value
    {
        std::unique_ptr<int> result;

        auto promise = Promise<std::unique_ptr<int>>{};
        promise.future().then(context, [&](std::unique_ptr<int> val) { result = std::move(val); });

        promise.fulfill(std::make_unique<int>(5));
        processEvents();

        ASSERT_TRUE(result != nullptr);
        ASSERT_TRUE(*result == 5);
    }
    // Free function as continuation
    {
        int result{};

        auto promise = Promise<int>{};
        promise.future().then(context, &twice).then(context, [&](int val) { result = val; });

        promise.fulfill(2);
        processEvents();

        ASSERT_TRUE(result == 4);
    }
}

TEST_F(FuturePromise, nestedFutures)
{
    nf::Context context;

    // Nested future is ready only when both futures are ready
    {
        int result = 0;

        auto promise1 = Promise<Future<std::unique_ptr<int>>>{};
        promise1.future().then(context, [&](std::unique_ptr<int> i) { result = *i; });

        auto promise2 = Promise<std::unique_ptr<int>>{};

        promise1.fulfill(Future{promise2});
        processEvents();

        ASSERT_FALSE(result == 2);

        auto v = std::make_unique<int>(2);
        promise2.fulfill(std::move(v));
        processEvents();

        ASSERT_TRUE(result == 2);
    }
    // Chain of nested futures
    {
        std::string result;
        auto p = std::make_unique<int>(0);

        execute(context, //
                [&]() { return execute(context, []() { return 1; }); })
            .then(context,
                  [&](int t) { return execute(context, [t]() { return std::to_string(t); }); })
            .then(context, [&, p = std::move(p)](const std::string &t) { result = t; });

        processEvents();

        ASSERT_EQ(result, "1");
    }
    // Unwrap
    {
        bool finished = false;

        auto f = [&]() -> Future<int> {
            return execute(context, []() { return 42; })
                .then(context,
                      [&](int i) {
                          return execute(
                              context, [](int x) { return x * 2; }, i);
                      })
                .unwrap(context);
        };

        f().then(context, [&](int /*unused*/) { finished = true; });

        processEvents();

        ASSERT_TRUE(finished);
    }
    // Unwrap void
    {
        bool finished = false;

        auto f = [&]() -> Future<void> {
            return execute(context, []() { return 42; })
                .then(context,
                      [&](int i) {
                          return execute(
                              context, []([[maybe_unused]] int x) -> void {}, i);
                      })
                .unwrap(context);
        };

        f().then(context, [&]() { finished = true; });

        processEvents();

        ASSERT_TRUE(finished);
    }
}

TEST_F(FuturePromise, optionalType)
{
    nf::Context context;

    // Fulfill with std::nullopt
    {
        std::optional<int> result{};

        Promise<std::optional<int>> promise{};
        nf::Executor::thisThread()->post([promise]() { promise.fulfill(std::nullopt); });
        promise.future().then(context, [&](auto res) { result = res; });

        processEvents();

        ASSERT_TRUE(result == std::nullopt);
    }
    // Fulfill with value
    {
        std::optional<int> result{};

        Promise<std::optional<int>> promise{};
        nf::Executor::thisThread()->post([promise]() { promise.fulfill(2); });
        promise.future().then(context, [&](auto res) { result = res; });

        processEvents();

        ASSERT_FALSE(result == std::nullopt);
        ASSERT_EQ(result, 2);
    }
}

TEST_F(FuturePromise, objectLifetime)
{
    // Continuation cleaned after execution
    {
        nf::Context context;
        auto p = std::make_shared<int>(0);
        execute(context, &twice, 2).then(context, [p](int /*unused*/) {}).then(context, [&]() {});

        ASSERT_EQ(p.use_count(), 2);
        ASSERT_FALSE(context.isEmpty());

        processEvents();

        ASSERT_EQ(p.use_count(), 1);
        ASSERT_TRUE(context.isEmpty());
    }
    // Continuation cleaned when shared state is killed
    {
        nf::Context context;
        auto p = std::make_shared<int>(0);
        {
            Promise<void> promise;
            promise.future().then(context, [p] {});
            EXPECT_THAT(p.use_count(), 2);
        }
        EXPECT_THAT(p.use_count(), 1);
    }
    // Continuation's lambda deleted immediately when context is reset
    {
        nf::Context context;
        auto p = std::make_shared<int>(0);

        Promise<void> promise;
        promise.future().then(context, [p] {});
        EXPECT_THAT(p.use_count(), 2);

        context.reset();
        EXPECT_THAT(p.use_count(), 1);
    }
    // Continuation's lambda deleted immediately when context is reset, nested future
    {
        nf::Context context;
        auto p = std::make_shared<int>(0);

        Promise<void> promise;
        Promise<int> otherPromise;
        promise.future()
            .then(context, [p, &otherPromise] { return otherPromise.future(); })
            .then(context, [p](int) {});
        EXPECT_THAT(p.use_count(), 3);

        promise.fulfill();

        processEvents();

        // First continuation should be gone
        EXPECT_THAT(p.use_count(), 2);

        context.reset();

        // Second continuation should be gone, too
        EXPECT_THAT(p.use_count(), 1);
    }
    // Continuation not called if context removed
    {
        auto context = std::make_unique<nf::Context>();
        bool shouldBeCalled{};
        bool shouldNotBeCalled{};

        execute(*context, //
                [&]() { shouldBeCalled = true; })
            .then(*context, [&]() { context.reset(); })
            .then(*context, [&]() { shouldNotBeCalled = true; });

        processEvents();

        ASSERT_TRUE(shouldBeCalled);

        processEvents();

        ASSERT_FALSE(shouldNotBeCalled);
    }
    // Continuation is called when Promise is deleted
    {
        nf::Context context;
        int result{};

        {
            auto promise = Promise<int>{};
            promise.future().then(context, [&](int val) { result = val; });
            promise.fulfill(5);
        }

        processEvents();

        ASSERT_EQ(result, 5);
    }
    // Switch context for continuation
    {
        auto context1 = std::make_unique<nf::Context>();
        auto context2 = std::make_unique<nf::Context>();
        bool shouldBeCalled{};

        execute(*context1, //
                []() {})
            .then(*context1, [&]() { context1.reset(); })
            .then(*context2, [&]() { shouldBeCalled = true; });

        processEvents();

        ASSERT_TRUE(shouldBeCalled);
    }
}

TEST_F(FuturePromise, threading)
{
    Context context;
    Promise<int> promise;

    int result = 0;

    /* Reply on TSAN to detect problems and fail this test. */
    auto thread = std::thread([promise] { promise.fulfill(50); });
    promise.future().then(context, [&](int value) { result = value; });

    thread.join();
    processEvents();

    ASSERT_EQ(50, result);
}

TEST_F(FuturePromise, valueType)
{
    static_assert(std::is_same_v<int, Future<int>::ValueType>);
    static_assert(std::is_same_v<void, Future<void>::ValueType>);
    static_assert(std::is_same_v<double, Future<Future<double>>::ValueType>);
}

TEST_F(FuturePromise, promiseType)
{
    using MyFuture = Future<int>;
    static_assert(std::is_same_v<Promise<int>, MyFuture::PromiseType>);
    static_assert(
        std::is_same_v<MyFuture, decltype(std::declval<MyFuture::PromiseType>().future())>);
}

TEST_F(FuturePromise, promiseCallOperator)
{
    nf::Context context;

    // No args
    {
        bool finished = false;
        auto promise = Promise<void>{};
        promise.future().then(context, [&]() { finished = true; });

        promise();
        processEvents();

        ASSERT_TRUE(finished);
    }
    // With one arg
    {
        std::string result;

        auto promise = Promise<std::string>{};
        promise.future().then(context, [&](std::string val) { result = std::move(val); });

        promise("gotcha!");
        processEvents();

        ASSERT_TRUE(result == "gotcha!");
    }

    // Promise used as callback
    {
        bool finished = false;
        auto promise = Promise<void>{};
        promise.future().then(context, [&]() { finished = true; });

        callPromise(promise);
        processEvents();

        ASSERT_TRUE(finished);
    }

    // Promise with one arg used as callback
    {
        std::string result;

        auto promise = Promise<std::string>{};
        promise.future().then(context, [&](std::string val) { result = std::move(val); });

        callPromise<std::string>(promise, "gotcha!");
        processEvents();

        ASSERT_TRUE(result == "gotcha!");
    }
}

TEST_F(FuturePromise, futureInStdAny)
{
    Context context;
    int result = 0;

    Promise<int> promise;
    std::any future = promise.future();
    std::any_cast<Future<int>>(future).then(context, [&result](int value) { result = value; });

    promise.fulfill(100);

    processEvents();
    EXPECT_EQ(100, result);
}

class MulticastPromiseTest : public Test
{
};


TEST_F(MulticastPromiseTest, verifyCompilation)
{
    nf::Context context;

    // MulticastPromise T
    {
        auto p = MulticastPromise<std::shared_ptr<int>>{};
        p.future().then(context,
                        [](const std::shared_ptr<int> & /*unused*/) -> std::shared_ptr<int> {
                            return {};
                        });
        p.future().then(context,
                        [](const std::shared_ptr<int> & /*unused*/) -> std::shared_ptr<int> {
                            return {};
                        });
        p.fulfill(std::make_shared<int>());
    }
    // MulticastPromise void
    {
        auto p = MulticastPromise<void>{};
        p.future().then(context, []() -> void {});
        p.future().then(context, []() -> void {});
        p.fulfill();
    }
}

TEST_F(MulticastPromiseTest, simpleContinuation)
{
    nf::Context context;

    // Multicast with void
    {
        size_t counter = 0;

        auto multicastPromise = MulticastPromise<void>{};
        multicastPromise.future().then(context, [&]() { counter++; });
        multicastPromise.future().then(context, [&]() { counter++; });
        multicastPromise.fulfill();
        multicastPromise.future().then(context, [&]() { counter++; });

        ASSERT_EQ(counter, 0);

        processEvents();

        ASSERT_EQ(counter, 3);
    }
    // Multicast copyable value
    {
        CopyTracer ct;
        auto sb = ct.sb(); // ct will be moved out

        bool finished = false;
        auto noop = [](CopyTracer && /*unused*/) {};
        auto finishLoop = [&](CopyTracer && /*unused*/) { finished = true; };
        auto multicastPromise = MulticastPromise<CopyTracer>{};
        multicastPromise.future().then(context, noop);
        multicastPromise.future().then(context, noop);
        multicastPromise.fulfill(std::move(ct));
        multicastPromise.future().then(context, finishLoop);

        // One copy per obtained future plus one for the multicast promise
        EXPECT_EQ(sb->cntCopied, 3);

        processEvents();

        ASSERT_TRUE(finished);
        ASSERT_EQ(sb->cntCopied, 3);
    }
    // MulticastPromise with free function as continuation
    {
        int result{};
        int result2{};

        auto multicastPromise = MulticastPromise<int>{};
        multicastPromise.future().then(context, &twice).then(context, [&](int val) {
            result = val;
        });
        multicastPromise.future().then(context, &twice).then(context, [&](int val) {
            result2 = val + 1;
        });

        multicastPromise.fulfill(2);

        processEvents();

        ASSERT_EQ(result, 4);
        ASSERT_EQ(result2, 5);
    }
    // MulticastPromise callback order is preserved
    {
        int accumulator = 1;

        bool finished = false;
        auto multicastPromise = MulticastPromise<void>{};
        multicastPromise.future().then(context, [&]() { //
            accumulator += 2;
        });
        multicastPromise.future().then(context, [&]() { //
            accumulator *= 2;
        });
        multicastPromise.fulfill();
        multicastPromise.future().then(context, [&]() { //
            accumulator += 7;
        });
        multicastPromise.future().then(context, [&]() { //
            finished = true;
        });

        processEvents();
        /* This should be (1 + 2) * 2 + 7 = 13 in the correct order
         * The following results are wrong:
         *  1 * 2 + 2 + 7 = 11
         *  (1 + 7) * 2 + 2 = 18
         *  (1 + 2 + 7) * 2 = 20
         */
        ASSERT_EQ(accumulator, 13);
    }
}

TEST_F(MulticastPromiseTest, sharedPointer)
{
    Context context;

    // All continuations will modify this original value
    auto value = std::make_shared<int>(0);

    MulticastPromise<std::shared_ptr<int>> promise;
    promise.fulfill(value);

    promise.future().then(context, [](auto val) { *val += 3; });
    promise.future().then(context, [](auto val) { *val *= 2; });

    processEvents();
    ASSERT_EQ(6, *value); // (0 + 3) * 2
}

TEST_F(MulticastPromiseTest, objectLifetime)
{
    // Multicast<T> continuation cleaned after execution
    {
        nf::Context context;
        auto p = std::make_shared<int>(0);
        MulticastPromise<int> multicastPromise{};

        multicastPromise.future().then(context, [p](int /*unused*/) {}).then(context, []() {});
        multicastPromise.future().then(context, [p](int /*unused*/) {}).then(context, []() {});

        ASSERT_EQ(p.use_count(), 3);
        multicastPromise.fulfill(2);

        processEvents();

        ASSERT_EQ(p.use_count(), 1);
    }
    // Multicast<void> continuation cleaned after execution
    {
        nf::Context context;
        auto p = std::make_shared<int>(0);
        MulticastPromise<void> multicastPromise{};

        multicastPromise.future().then(context, [p]() {}).then(context, []() {});
        multicastPromise.future().then(context, [p]() {}).then(context, []() {});

        ASSERT_EQ(p.use_count(), 3);
        multicastPromise.fulfill();

        processEvents();

        ASSERT_EQ(p.use_count(), 1);
    }
    // Continuation's lambda deleted immediately when context is reset
    {
        nf::Context context;
        auto p = std::make_shared<int>(0);
        MulticastPromise<void> multicastPromise{};

        multicastPromise.future().then(context, [p]() {}).then(context, []() {});
        multicastPromise.future().then(context, [p]() {}).then(context, []() {});

        EXPECT_THAT(p.use_count(), 3);
        context.reset();

        EXPECT_THAT(p.use_count(), 1);
    }
    // Continuation is called when MulticastPromise is deleted
    {
        nf::Context context;
        int result{};
        int result2{};

        {
            auto multicastPromise = MulticastPromise<int>{};
            multicastPromise.future().then(context, [&](int val) { result = val; });
            multicastPromise.future().then(context, [&](int val) { result2 = val; });
            multicastPromise.fulfill(5);
        }

        processEvents();

        ASSERT_EQ(result, 5);
        ASSERT_EQ(result2, 5);
    }
}

TEST_F(MulticastPromiseTest, threading)
{
    Context context;
    MulticastPromise<int> promise;

    int result = 0;

    /* Reply on TSAN to detect problems and fail this test. */
    auto thread = std::thread([promise] { promise.fulfill(50); });
    promise.future().then(context, [&](int value) { result += value; });
    promise.future().then(context, [&](int value) { result += value; });

    thread.join();
    processEvents();

    ASSERT_EQ(100, result);
}

class Composition : public Test
{
};

TEST_F(Composition, whenAny)
{
    nf::Context context;

    // Two futures, different types
    {
        auto p1 = Promise<int>{};
        auto p2 = Promise<std::string>{};

        auto oneDone = whenAny(context, p1.future(), p2.future());
        oneDone.then(context, [&](const std::variant<int, std::string> &variant) {
            std::visit(boost::hana::overload([](int v) { ASSERT_TRUE(v == 42); },
                                             [](const auto & /*unused*/) { FAIL(); }),
                       variant);
        });

        p1.fulfill(42);

        processEvents();
    }
    // Three futures, same type types
    {
        auto p1 = Promise<std::string>{};
        auto p2 = Promise<int>{};
        auto p3 = Promise<std::string>{};

        auto oneDone = whenAny(context, p1.future(), p2.future(), p3.future());
        oneDone.then(context, [&](auto variant) {
            auto *expectedStr = std::get_if<2>(&variant);
            ASSERT_TRUE(expectedStr != nullptr);
            ASSERT_TRUE(*expectedStr == "third future");
        });

        p3.fulfill("third future");

        processEvents();
    }
    // Two futures, multiple promises fulfilled
    {
        auto p1 = Promise<std::unique_ptr<int>>{};
        auto p2 = Promise<int>{};

        auto oneDone = whenAny(context, p1.future(), p2.future());
        oneDone.then(context, [&](auto variant) {
            std::visit(boost::hana::overload([](std::unique_ptr<int> v) { ASSERT_TRUE(*v == 23); },
                                             [](int /*unused*/) { FAIL(); }),
                       std::move(variant));
        });

        p1.fulfill(std::make_unique<int>(23));
        p2.fulfill(42);

        processEvents();
    }
    // Two futures, void promise
    {
        auto p1 = Promise<int>{};
        auto p2 = Promise<void>{};

        auto oneDone = whenAny(context, p1.future(), p2.future());
        oneDone.then(context, [&](auto variant) {
            std::visit(boost::hana::overload([](auto /*unused*/) {},
                                             [](int /*unused*/) { FAIL(); }),
                       std::move(variant));
        });

        p2.fulfill();

        processEvents();
    }
}

TEST_F(Composition, whenAll)
{
    nf::Context context;

    // Two futures, different types
    {
        auto p1 = Promise<int>{};
        auto p2 = Promise<std::string>{};

        auto allDone = whenAll(context, p1.future(), p2.future());
        allDone.then(context, [&](std::tuple<int, std::string> tuple) {
            ASSERT_TRUE(std::get<0>(tuple) == 42);
            ASSERT_TRUE(std::get<1>(tuple) == "foo");
        });

        p1.fulfill(42);
        p2.fulfill("foo");

        processEvents();
    }
    // Three futures, same type types
    {
        auto p1 = Promise<std::string>{};
        auto p2 = Promise<int>{};
        auto p3 = Promise<std::string>{};

        auto allDone = whenAll(context, p1.future(), p2.future(), p3.future());
        allDone.then(context, [&](auto tuple) {
            ASSERT_TRUE(std::get<0>(tuple) == "first future");
            ASSERT_TRUE(std::get<1>(tuple) == 42);
            ASSERT_TRUE(std::get<2>(tuple) == "third future");
        });

        p3.fulfill("third future");
        p2.fulfill(42);
        p1.fulfill("first future");

        processEvents();
    }
    // Two futures, void promise
    {
        auto p1 = Promise<int>{};
        auto p2 = Promise<void>{};

        auto allDone = whenAll(context, p1.future(), p2.future());
        allDone.then(context, [&](auto tuple) {
            ASSERT_TRUE(std::get<0>(tuple) == 42);
            ASSERT_TRUE(std::get<1>(tuple) == std::monostate{});
        });

        p1.fulfill(42);
        p2.fulfill();

        processEvents();
    }
    // Two futures, void promise - unfulfilled
    {
        auto p1 = Promise<int>{};
        auto p2 = Promise<std::unique_ptr<int>>{};

        auto allDone = whenAll(context, p1.future(), p2.future());
        allDone.then(context, [&](auto /*tuple*/) { FAIL(); });

        p2.fulfill(std::make_unique<int>(23));

        processEvents();
    }
}

TEST_F(Composition, nesting)
{
    nf::Context context;

    // Two futures, whenAny
    {
        auto p1 = Promise<int>{};
        auto p2 = Promise<void>{};
        auto p3 = Promise<std::unique_ptr<int>>{};

        auto received = std::variant<int, std::variant<std::monostate, std::unique_ptr<int>>>{};

        auto oneDone = whenAny(context, p1.future(), whenAny(context, p2.future(), p3.future()));
        oneDone.then(context, [&](auto variant) { received = std::move(variant); });

        p3.fulfill(std::make_unique<int>(23));

        processEvents();

        std::visit(boost::hana::overload(
                       [](auto innerVariant) {
                           std::visit(boost::hana::overload([](auto /*unused*/) { FAIL(); },
                                                            [](std::unique_ptr<int> value) {
                                                                ASSERT_EQ(*value, 23);
                                                            }),
                                      std::move(innerVariant));
                       },
                       [](int /*unused*/) { FAIL(); }),
                   std::move(received));
    }
    // Two futures, whenAll
    {
        auto p1 = Promise<int>{};
        auto p2 = Promise<void>{};
        auto p3 = Promise<std::unique_ptr<int>>{};

        auto received = std::tuple<int, std::tuple<std::monostate, std::unique_ptr<int>>>{};

        auto allDone = whenAll(context, p1.future(), whenAll(context, p2.future(), p3.future()));
        allDone.then(context, [&](auto tuple) { received = std::move(tuple); });

        p1.fulfill(42);
        p2.fulfill();
        p3.fulfill(std::make_unique<int>(23));

        processEvents();

        ASSERT_TRUE(std::get<0>(received) == 42);
        auto &innerTuple = std::get<1>(received);
        ASSERT_TRUE(std::get<0>(innerTuple) == std::monostate{});
        ASSERT_TRUE(*std::get<1>(innerTuple) == 23);
    }

    // Two futures, whenAll/whenAny
    {
        auto p1 = Promise<int>{};
        auto p2 = Promise<void>{};
        auto p3 = Promise<std::unique_ptr<int>>{};

        auto received = std::tuple<int, std::variant<std::monostate, std::unique_ptr<int>>>{};

        auto allDone = whenAll(context, p1.future(), whenAny(context, p2.future(), p3.future()));
        allDone.then(context, [&](auto tuple) { received = std::move(tuple); });

        p1.fulfill(42);
        p3.fulfill(std::make_unique<int>(23));

        processEvents();
        ASSERT_EQ(std::get<0>(received), 42);
        auto &innerVariant = std::get<1>(received);
        std::visit(boost::hana::overload([](auto /*unused*/) { FAIL(); },
                                         [&](std::unique_ptr<int> value) {
                                             ASSERT_EQ(*value, 23);
                                         }),
                   std::move(innerVariant));
    }
}

TEST_F(Composition, timeout)
{
    nf::Context context;

    bool called = false;
    auto timeout = makeTimeout(context, 200ms);

    timeout.then(context, [&](auto /*unused*/) { called = true; });
    processEvents(25ms);
    ASSERT_FALSE(called);

    processEvents(200ms);
    ASSERT_TRUE(called);
}

TEST_F(Composition, expiresIn)
{
    nf::Context context;

    // Promise<void> that doesn't expires
    {
        auto p = Promise<void>{};
        auto futureWithTimeout = expiresIn(context, 100ms, p.future());
        futureWithTimeout.then(context, [&](auto variant) {
            std::visit(boost::hana::overload([](const std::monostate & /*unused*/) {},
                                             [](const Timeout & /*unused*/) { FAIL(); }),
                       variant);
        });

        p.fulfill();

        processEvents();
    }
    // Promise<void> that expires
    {
        auto p = Promise<void>{};
        bool expired = false;

        auto futureWithTimeout = expiresIn(context, 100ms, p.future());
        futureWithTimeout.then(context, [&](auto variant) {
            std::visit(boost::hana::overload([&](const Timeout & /*unused*/) { expired = true; },
                                             [](const std::monostate & /*unused*/) { FAIL(); }),
                       variant);
        });

        processEvents(150ms);
        ASSERT_TRUE(expired);
    }
    // Promise<int> that doesn't expires
    {
        auto p = Promise<int>{};

        auto futureWithTimeout = expiresIn(context, 100ms, p.future());
        futureWithTimeout.then(context, [&](auto variant) {
            std::visit(boost::hana::overload([](int /*unused*/) {},
                                             [](const Timeout & /*unused*/) { FAIL(); }),
                       variant);
        });

        p.fulfill(42);

        processEvents();
    }
}

TEST_F(Composition, untilDone)
{
    Context context;

    auto createOptional = [](bool cond,
                             auto &&value) -> std::optional<std::decay_t<decltype(value)>> {
        if (cond) {
            return std::forward<decltype(value)>(value);
        }
        return std::nullopt;
    };

    // Sync function with unique optional return type
    {
        int i{};
        auto p = std::make_shared<int>(0);
        auto process = [&, p] {
            EXPECT_THAT(p.use_count(), 3);
            ++i;
            return createOptional(i == 10, std::make_unique<std::string>(std::to_string(i)));
        };

        EXPECT_THAT(p.use_count(), 2);

        untilDone(context, std::move(process)).then(context, [&, p](auto result) {
            EXPECT_THAT(p.use_count(), 2);
            ASSERT_THAT(result, NotNull());
            EXPECT_THAT(*result, "10");
        });

        EXPECT_THAT(p.use_count(), 3);

        processEvents();

        EXPECT_THAT(p.use_count(), 1);
        ASSERT_TRUE(i != 0);
    }

    // Async function with unique optional return type
    {
        int i{};
        auto p = std::make_shared<int>(0);
        auto process = [&, p] {
            EXPECT_THAT(p.use_count(), 3);
            ++i;
            return async::execute(
                context,
                [&, p](int j) {
                    EXPECT_THAT(p.use_count(), 4);
                    return createOptional(j == 10,
                                          std::make_unique<std::string>(std::to_string(j)));
                },
                i);
        };

        EXPECT_THAT(p.use_count(), 2);

        untilDone(context, std::move(process)).then(context, [&, p](auto result) {
            EXPECT_THAT(p.use_count(), 2);
            ASSERT_THAT(result, NotNull());
            EXPECT_EQ("10", *result);
        });

        EXPECT_THAT(p.use_count(), 3);

        processEvents();

        EXPECT_THAT(p.use_count(), 1);
        ASSERT_TRUE(i != 0);
    }

    // Premature termination of sync function iteration due to context destruction
    {
        int i{};
        auto p = std::make_shared<int>(0);
        auto process = [&, p] {
            EXPECT_THAT(p.use_count(), 3);
            ++i;
            if (i == 5) {
                context.reset();
            }
            return createOptional(i == 10, i);
        };

        EXPECT_THAT(p.use_count(), 2);

        untilDone(context, std::move(process)).then(context, [&, p](auto) {
            FAIL() << "Continuation should not be executed after context destruction";
        });

        EXPECT_THAT(p.use_count(), 3);

        processEvents();
        context.reset();

        EXPECT_THAT(p.use_count(), 1);
        ASSERT_EQ(5, i);
    }

    // Premature termination of async function iteration due to context destruction
    {
        int i{};
        auto p = std::make_shared<int>(0);
        auto process = [&, p] {
            EXPECT_THAT(p.use_count(), 3);
            ++i;
            if (i == 5) {
                context.post([&context] { context.reset(); });
            }
            return async::execute(
                context,
                [&, p](int j) {
                    EXPECT_THAT(p.use_count(), 4);
                    // Context deletion should have happened before this is executed with j == 5
                    EXPECT_THAT(j, ::testing::Lt(5));
                    return createOptional(j == 10,
                                          std::make_unique<std::string>(std::to_string(j)));
                },
                i);
        };

        EXPECT_THAT(p.use_count(), 2);

        untilDone(context, std::move(process)).then(context, [&, p](auto) {
            FAIL() << "Continuation should not be executed after context destruction";
        });

        EXPECT_THAT(p.use_count(), 3);

        processEvents();

        EXPECT_THAT(p.use_count(), 1);
        ASSERT_EQ(5, i);
    }
}

class ExecuteOnThreadTest : public Test
{
public:
    ExecuteOnThreadTest() noexcept
        : Test(AsioBackend{})
    {
    }
};

TEST_F(ExecuteOnThreadTest, executeOnThread)
{
    std::thread::id threadId{};
    auto executor = Executor::thisThread();

    Context context;
    Thread thread;

    int result = 0;

    executor->post([&] {
        auto future = execute(
            thread,
            [&](int value) {
                /* The task is invoked in a worker thread. */
                threadId = std::this_thread::get_id();
                return value + 2;
            },
            40);
        future.then(context, [&](int value) {
            /* The continuation is invoked in the main thread. */
            result = value;
            executor->stop();
        });
    });
    executor->run();
    thread.syncJoin();

    ASSERT_EQ(42, result);
    ASSERT_NE(std::this_thread::get_id(), threadId);
}

class FutureResultTest : public Test
{
};


TEST_F(FutureResultTest, basicOperations)
{
    Context context;
    // success
    {
        int endValue = 0;
        auto p1 = Promise<Result<int>>{};
        auto f1 = p1.future();

        f1.whenOk(context, [&](int v) -> Result<int> {
            endValue = v;
            return {31};
        });

        p1.fulfill(42);
        processEvents();

        ASSERT_EQ(42, endValue);
    }
    // success void
    {
        int endValue = 0;
        auto p1 = Promise<Result<void>>{};
        auto f1 = p1.future();

        f1.whenOk(context, [&]() -> Result<int> {
            endValue = 42;
            return {31};
        });

        p1.fulfill({});
        processEvents();

        ASSERT_EQ(42, endValue);
    }
    // failure
    {
        int endValue = 0;
        auto p1 = Promise<Result<int>>{};
        auto f1 = p1.future();

        f1.whenOk(context, [&](int v) -> Result<int> {
            endValue = v;
            return {31};
        });

        p1.fulfill(result::Err{});
        processEvents();

        ASSERT_EQ(0, endValue);
    }
    // failure void
    {
        int endValue = 0;
        auto p1 = Promise<Result<void>>{};
        auto f1 = p1.future();

        f1.whenOk(context, [&]() -> Result<int> {
            endValue = 42;
            return {31};
        });

        p1.fulfill(result::Err{});
        processEvents();

        ASSERT_EQ(0, endValue);
    }
    // ok, then failure
    {
        bool hasError = false;
        auto p1 = Promise<Result<int>>{};
        auto f1 = p1.future();

        f1.whenOk(context, [&](int) -> Result<int> {
              return nf::result::Err{};
          }).then(context, [&](auto r) { hasError = r.hasError(); });

        p1.fulfill(42);
        processEvents();

        ASSERT_TRUE(hasError);
    }
    // nested
    {
        int endValue = 0;
        auto p1 = Promise<Future<Result<int>>>{};
        auto f1 = p1.future();

        f1.whenOk(context, [&](int v) -> Result<int> {
            endValue = v;
            return 31;
        });

        p1.fulfill(Future<Result<int>>{42});
        processEvents();
        ASSERT_EQ(42, endValue);
    }
    // ok, value type change
    {
        std::string endValue;
        auto p1 = Promise<Result<int>>{};
        auto f1 = p1.future();

        f1.whenOk(context, [](int v) -> Result<std::string> {
              return std::to_string(v);
          }).then(context, [&](auto r) { endValue = r.valueOr(""); });

        p1.fulfill(Result<int>{42});
        processEvents();
        ASSERT_EQ("42", endValue);
    }
    // move-only types
    {
        std::unique_ptr<std::string> endValue;
        auto p1 = Promise<Result<MoveTracer, int>>{};
        auto f1 = p1.future();

        f1.whenOk(context, [](const MoveTracer &) {
              return Result<std::unique_ptr<std::string>, int>{
                  std::make_unique<std::string>("test")};
          }).then(context, [&](auto r) { endValue = std::move(r).value(); });

        p1.fulfill(MoveTracer{});
        processEvents();
        ASSERT_EQ("test", *endValue);
    }
}

Result<std::string, int> op1()
{
    return result::Ok{std::string{"42"}};
}

Result<int, int> op2(const std::string &s)
{
    try {
        return std::stoi(s);
    } catch (const std::exception &) {
        return result::Err{42};
    }
}

Future<Result<std::unique_ptr<int>, int>> op3(int v)
{
    return {std::make_unique<int>(v)};
}

TEST_F(FutureResultTest, complexChain)
{
    Context context;

    int endValue = 0;

    async::execute(context, &op1)
        .whenOk(context,
                [](const std::string &v) -> Result<int, int> {
                    if (v.empty()) {
                        return result::Err{42};
                    }
                    return op2(v);
                })
        .whenOk(context, &op3)
        .then(context, [&](auto result) {
            if (!result) {
                return;
            }
            endValue = *result.value();
        });

    processEvents();
    ASSERT_EQ(42, endValue);
}
