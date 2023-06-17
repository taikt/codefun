/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include <nf/Singleton.h>
#include <nf/testing/Test.h>

class SingletonTest : public nf::testing::Test
{
public:
    ~SingletonTest() override
    {
        nf::SingletonRegistry::releaseAll();
    }

    class SingletonableClass
    {
    public:
        explicit SingletonableClass(SingletonTest *test)
            : m_test(test)
        {
        }

        ~SingletonableClass()
        {
            if (m_test) {
                m_test->m_isDestroyed = true;
            }
        }

        SingletonableClass &operator=(SingletonableClass &&other) noexcept
        {
            m_test = other.m_test;
            other.m_test = nullptr;
            return *this;
        }

    private:
        SingletonTest *m_test;
    };

protected:
    bool m_isDestroyed{false};
};

/*
 * Check if SingletonRegistry::construct() and releaseAll() work.
 */
TEST_F(SingletonTest, singletonRegistry)
{
    auto instance = nf::SingletonRegistry::construct<SingletonableClass>(this);

    /* An instance is still being hold by the registry. */
    instance.reset();
    EXPECT_FALSE(m_isDestroyed);

    nf::SingletonRegistry::releaseAll();
    EXPECT_TRUE(m_isDestroyed);
}

/*
 * Check if SingletonHolder works if all client instances are released before releaseAll()
 * is invoked.
 */
TEST_F(SingletonTest, singletonHolder_beforeReleaseAll)
{
    nf::SingletonHolder<SingletonableClass> holder(this);

    auto instance = holder.lock();

    /* An instance is still being held by the registry. */
    instance.reset();
    EXPECT_FALSE(m_isDestroyed);

    /* We still can get an instance via a factory method. */
    instance = holder.lock();
    EXPECT_NE(nullptr, instance);

    instance.reset();
    EXPECT_FALSE(m_isDestroyed);
    nf::SingletonRegistry::releaseAll();
    EXPECT_TRUE(m_isDestroyed);

    /* Once the singleton registry is cleaned, the factory method shall return nullptr. */
    instance = holder.lock();
    EXPECT_EQ(nullptr, instance);
}

/*
 * Check if SingletonHolder works if releaseAll() is called before all client instances are
 * released.
 */
TEST_F(SingletonTest, singletonHolder_afterReleaseAll)
{
    nf::SingletonHolder<SingletonableClass> holder(this);

    auto instance = holder.lock();

    /* Release all entries from the registry. The instance is NOT destroyed because a client
     * code still holds a reference. */
    nf::SingletonRegistry::releaseAll();
    EXPECT_FALSE(m_isDestroyed);

    /* It is still possible to get the reference via a factory method because a client code
     * still holds a reference. */
    instance = holder.lock();
    EXPECT_NE(nullptr, instance);

    /* When the last reference is gone, the instance shall be destroyed. */
    instance.reset();
    EXPECT_TRUE(m_isDestroyed);

    /* Now the factory method shall return nullptr. */
    instance = holder.lock();
    EXPECT_EQ(nullptr, instance);
}

TEST_F(SingletonTest, logTest)
{
    nf::SingletonHolder<SingletonableClass> holder(this);

    /* Release all entries from the registry*/
    nf::SingletonRegistry::releaseAll();

    /* Now the factory method shall return nullptr
     * and log the name of the instance*/
    auto instance = holder.lock();
    ASSERT_EQ(nullptr, instance);
    EXPECT_THAT(logMessages().back(),
                ::testing::MatchesRegex(
                    "An instance of the singleton object "
                    "SingletonTest::SingletonableClass has been already deleted!"));
}
