/*
 * BMW Neo Framework
 *
 * Copyright (C) 2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include <nf/Context.h>
#include <nf/ContextItem.h>
#include <nf/testing/Test.h>

#include <functional>

using namespace nf;
using namespace nf::testing;

class ContextItemTest : public Test
{
};

/* BaseItem is only needed to test that item hierarchy works as expected, e.g. needed methods
 * are virtual etc. */
class BaseItem : public ContextItem
{
public:
    void decontextualize()
    {
        ContextItem::decontextualize();
    }
};

class TestItem : public BaseItem
{
public:
    explicit TestItem(std::function<void()> &&onInvoked = nullptr,
                      std::function<void()> &&onDestroy = nullptr)
        : m_onInvoked(std::move(onInvoked))
        , m_onDestroy(std::move(onDestroy))
    {
    }

    ~TestItem() override
    {
        if (m_onDestroy) {
            m_onDestroy();
        }
    }

    void directInvoke()
    {
        /* Decontextualization might destroy this TestItem object if it was referenced only
         * by the Context it is bound to. */
        m_onInvoked();
        decontextualize();
    }

    void deferredInvoke()
    {
        post([this] {
            /* This shall not delete this TestItem object because of post(). It shall be safe
             * to access local variables after the decontextualization is done. */
            decontextualize();
            m_onInvoked();
        });
    }

private:
    std::function<void()> m_onInvoked;
    std::function<void()> m_onDestroy;
};

TEST_F(ContextItemTest, contextEmpty)
{
    const Context context;
    auto item = std::make_shared<TestItem>();

    EXPECT_TRUE(context.isEmpty());
    context.bind(item);
    EXPECT_EQ(2, item.use_count());
    EXPECT_FALSE(context.isEmpty());
    item->decontextualize();
    EXPECT_TRUE(context.isEmpty());
}

TEST_F(ContextItemTest, itemDestroyed)
{
    bool isDestroyed = false;

    std::unique_ptr<BaseItem> item =
        std::make_unique<TestItem>(nullptr, [&isDestroyed] { isDestroyed = true; });

    EXPECT_FALSE(isDestroyed);
    item.reset();
    EXPECT_TRUE(isDestroyed);
}

TEST_F(ContextItemTest, contextDestroyed)
{
    bool isDestroyed = false;

    Context context;
    auto item = std::make_shared<TestItem>(nullptr, [&isDestroyed] { isDestroyed = true; });
    context.bind(item);
    item.reset();

    EXPECT_FALSE(isDestroyed);
    context.reset();
    EXPECT_TRUE(isDestroyed);
}

TEST_F(ContextItemTest, alreadyBound)
{
    Context context1;
    Context context2;

    auto item = std::make_shared<TestItem>();
    context1.bind(item);

    auto noCoreDumps = disableCoreDumps();
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    ASSERT_DEATH({ context2.bind(item); }, "");
}

TEST_F(ContextItemTest, destroyedInDecontextualize_deferred)
{
    bool isInvoked = false;

    const Context context;
    auto item = std::make_shared<TestItem>([&isInvoked] { isInvoked = true; });
    context.bind(item);
    item->deferredInvoke();
    item.reset();

    EXPECT_FALSE(context.isEmpty());
    EXPECT_FALSE(isInvoked);
    processEvents();
    EXPECT_TRUE(context.isEmpty());
    EXPECT_TRUE(isInvoked);
}

TEST_F(ContextItemTest, keepAliveContext_deferred)
{
    bool isInvoked = false;

    auto context = std::make_shared<Context>();
    auto item = std::make_shared<TestItem>([&isInvoked, context]() mutable {
        isInvoked = true;
        /* It is okay to explicitly reset the context here because this function is invoked
         * from ContextItem::post() and it holds a shared pointer to ContextState. */
        context.reset();
    });
    context->bind(item);
    item->deferredInvoke();

    context.reset();
    item.reset();

    EXPECT_FALSE(isInvoked);
    processEvents();
    EXPECT_TRUE(isInvoked);
}

TEST_F(ContextItemTest, destroyedInDecontextualize_direct)
{
    bool isInvoked = false;

    const Context context;
    auto item = std::make_shared<TestItem>([&isInvoked] { isInvoked = true; });
    context.bind(item);

    auto *rawItem = item.get();
    item.reset();

    EXPECT_FALSE(context.isEmpty());
    EXPECT_FALSE(isInvoked);

    // TestItem is still valid because it is kept alive by the Context
    rawItem->directInvoke();

    EXPECT_TRUE(context.isEmpty());
    EXPECT_TRUE(isInvoked);
}

TEST_F(ContextItemTest, keepAliveContext_direct)
{
    bool isInvoked = false;

    auto context = std::make_shared<Context>();
    auto item = std::make_shared<TestItem>([&isInvoked, context]() mutable {
        isInvoked = true;
        /* It is NOT okay to explicitly delete the context here as it WILL delete `this` which
         * would make it impossible to decontextualize TestItem. The context, however, will be
         * deleted later during the decontextualization and deletion of TestItem. */
    });
    context->bind(item);

    auto *rawItem = item.get();
    context.reset();
    item.reset();

    EXPECT_FALSE(isInvoked);
    rawItem->directInvoke();
    EXPECT_TRUE(isInvoked);
}
