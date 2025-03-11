/*
 * BMW Neo Framework
 *
 * Copyright (C) 2021 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include <nf/Framework.h>
#include <nf/RwLock.h>
#include <nf/Thread.h>
#include <nf/testing/Test.h>

#include <condition_variable>

using namespace nf;
using namespace nf::testing;

class RwLockTest : public nf::testing::Test
{
};

class RwLockThreadTest : public nf::testing::Test
{
public:
    RwLockThreadTest() noexcept
        : Test(NoFramework{})
    {
        Framework::initialize(std::make_unique<AsioFramework>());
    }
};

TEST_F(RwLockTest, read)
{
    RwLock lock;

    bool isInvoked = false;
    auto read = lock.lockRead([&] { isInvoked = true; });

    EXPECT_FALSE(isInvoked);

    processEvents();
    EXPECT_TRUE(isInvoked);
}

TEST_F(RwLockTest, write)
{
    RwLock lock;

    bool isInvoked = false;
    auto write = lock.lockWrite([&] { isInvoked = true; });

    EXPECT_FALSE(isInvoked);

    processEvents();
    EXPECT_TRUE(isInvoked);
}

TEST_F(RwLockTest, twoReads)
{
    RwLock lock;

    int cntInvoked = 0;
    auto read1 = lock.lockRead([&] { cntInvoked++; });
    auto read2 = lock.lockRead([&] { cntInvoked++; });

    EXPECT_EQ(0, cntInvoked);

    processEvents();
    EXPECT_EQ(2, cntInvoked);
}

TEST_F(RwLockTest, twoReadAndWrite)
{
    RwLock lock;

    int cntReadInvoked = 0;
    bool isWriteInvoked = false;
    auto read1 = lock.lockRead([&] { cntReadInvoked++; });
    auto write = lock.lockWrite([&] { isWriteInvoked = true; });
    auto read2 = lock.lockRead([&] { cntReadInvoked++; });

    EXPECT_FALSE(cntReadInvoked > 0 || isWriteInvoked);

    processEvents();
    EXPECT_EQ(2, cntReadInvoked);
    EXPECT_FALSE(isWriteInvoked);

    read1.reset();
    processEvents();
    EXPECT_FALSE(isWriteInvoked);

    read2.reset();
    processEvents();
    EXPECT_TRUE(isWriteInvoked);
}

TEST_F(RwLockTest, writeAndRead)
{
    RwLock lock;

    bool isReadInvoked = false;
    bool isWriteInvoked = false;
    auto write = lock.lockWrite([&] { isWriteInvoked = true; });
    auto read = lock.lockRead([&] { isReadInvoked = true; });

    EXPECT_FALSE(isReadInvoked || isWriteInvoked);

    processEvents();
    EXPECT_FALSE(isReadInvoked);
    EXPECT_TRUE(isWriteInvoked);

    write.reset();
    processEvents();
    EXPECT_TRUE(isReadInvoked);
}

TEST_F(RwLockTest, unsavedToken)
{
    RwLock lock;

    bool isReadInvoked = false;
    bool isWriteInvoked = false;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-result"
    lock.lockRead([&] { isReadInvoked = true; });
#pragma GCC diagnostic pop

    auto write = lock.lockWrite([&] { isWriteInvoked = true; });

    EXPECT_FALSE(isReadInvoked || isWriteInvoked);

    processEvents();
    EXPECT_FALSE(isReadInvoked);
    EXPECT_TRUE(isWriteInvoked);
}

TEST_F(RwLockThreadTest, callbackInvokedInThread)
{
    Thread thread;

    RwLock lock;
    bool isInvoked = false;

    std::mutex mutex;
    std::condition_variable cv;
    std::thread::id threadId;

    RaiiToken read;
    thread.post([&] {
        read = lock.lockRead([&] {
            std::unique_lock ulock(mutex);
            threadId = std::this_thread::get_id();
            isInvoked = true;
            ulock.unlock();
            cv.notify_all();
        });
    });

    std::unique_lock ulock(mutex);
    cv.wait(ulock, [&] { return isInvoked; });

    thread.syncJoin();

    EXPECT_TRUE(isInvoked);
    EXPECT_NE(std::this_thread::get_id(), threadId);
}
