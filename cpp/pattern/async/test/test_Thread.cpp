/*
 * BMW Neo Framework
 *
 * Copyright (C) 2020-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include <nf/Executor.h>
#include <nf/Framework.h>
#include <nf/Logging.h>
#include <nf/Thread.h>
#include <nf/testing/Test.h>

#include <chrono>
#include <condition_variable>
#include <string>
#include <thread>

#define LOG_TEST(__msg) nf::info("*** {}", __msg)

using namespace nf;
using namespace nf::testing;
using namespace std::chrono_literals;

class ThreadTest : public Test
{
public:
    ThreadTest() noexcept;

public:
    void notifyStarted()
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        LOG_TEST("Notifying the thread is started!");
        m_isStarted = true;
        lock.unlock();
        m_isStartedCv.notify_one();
    }

    void waitUntilStarted()
    {
        LOG_TEST("Waiting until the thread is started...");
        std::unique_lock<std::mutex> lock(m_mutex);
        bool isStarted = m_isStartedCv.wait_for(lock, 2s, [this]() { return m_isStarted; });
        LOG_TEST("...done!");
        ASSERT_TRUE(isStarted);
        m_isStarted = false;
    }

protected:
    bool m_isStarted{false};
    std::condition_variable m_isStartedCv;
    std::mutex m_mutex;
};

ThreadTest::ThreadTest() noexcept
    : Test(NoFramework{})
{
    Framework::initialize(std::make_unique<AsioFramework>());
}

TEST_F(ThreadTest, singlePost)
{
    std::thread::id threadId{};

    Thread thread;
    thread.post([&] {
        threadId = std::this_thread::get_id();
        notifyStarted();
    });

    waitUntilStarted();
    thread.syncJoin();

    ASSERT_NE(std::this_thread::get_id(), threadId);
}

TEST_F(ThreadTest, sequentialPost)
{
    std::thread::id threadId{};

    Thread thread;
    thread.post([&] {
        threadId = std::this_thread::get_id();
        notifyStarted();
    });

    waitUntilStarted();

    bool isExecuted{false};
    thread.post([&] {
        isExecuted = true;
        ASSERT_EQ(std::this_thread::get_id(), threadId);
    });
    thread.syncJoin();

    ASSERT_TRUE(isExecuted);
    ASSERT_NE(std::this_thread::get_id(), threadId);
}

TEST_F(ThreadTest, concurrentPosts)
{
    int value{0};
    Thread thread;
    for (int i = 0; i < 10; i++) {
        thread.post([&, i = i] {
            ASSERT_EQ(i, value);
            value = i + 1;
        });
    }
    thread.syncJoin();
}

TEST_F(ThreadTest, immediateSyncJoin)
{
    Thread thread;
    thread.syncJoin();
}

TEST_F(ThreadTest, asyncJoin)
{
    std::thread::id threadId{};
    auto executor = Executor::mainThread();

    Thread thread;
    thread.post([&] { notifyStarted(); });

    executor->post([&] {
        waitUntilStarted();
        thread.join([&] {
            threadId = std::this_thread::get_id();
            executor->stop();
        });
    });
    executor->run();

    ASSERT_EQ(std::this_thread::get_id(), threadId);
}

TEST_F(ThreadTest, assert_destroyWhileStarting)
{
    auto noCoreDumps = disableCoreDumps();
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    ASSERT_DEATH({ Thread thread; }, "");
}

TEST_F(ThreadTest, assert_destroyWhileRunning)
{
    auto noCoreDumps = disableCoreDumps();
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    ASSERT_DEATH(
        {
            Thread thread;
            thread.post([this] { notifyStarted(); });
            waitUntilStarted();
        },
        "");
}

TEST_F(ThreadTest, assert_throwIntFromExecutor)
{
    auto noCoreDumps = disableCoreDumps();
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    ASSERT_DEATH(
        {
            Thread thread;
            thread.post([this] {
                notifyStarted();
                // NOLINTNEXTLINE(hicpp-exception-baseclass)
                throw 42;
            });
            waitUntilStarted();
            thread.syncJoin();
        },
        "");
}

TEST_F(ThreadTest, assert_throwStdExceptionFromExecutor)
{
    auto noCoreDumps = disableCoreDumps();
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    ASSERT_DEATH(
        {
            Thread thread;
            thread.post([this] {
                notifyStarted();
                throw std::runtime_error("die, die, die");
            });
            waitUntilStarted();
            thread.syncJoin();
        },
        "");
}

TEST_F(ThreadTest, assert_destroyWhileJoining)
{
    auto noCoreDumps = disableCoreDumps();
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    ASSERT_DEATH(
        {
            Thread thread;
            thread.post([this] { notifyStarted(); });
            waitUntilStarted();
            thread.join([] {});
        },
        "");
}

TEST_F(ThreadTest, assert_postAfterJoin)
{
    auto noCoreDumps = disableCoreDumps();
    Thread thread;
    thread.syncJoin();
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    ASSERT_DEATH({ thread.post([] {}); }, "");
}

TEST_F(ThreadTest, assert_postFromWrongThread)
{
    auto noCoreDumps = disableCoreDumps();
    Thread thread;
    thread.post([&] {
        notifyStarted();
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
        ASSERT_DEATH({ thread.post([] {}); }, "");
    });
    waitUntilStarted();
    thread.syncJoin();
}

TEST_F(ThreadTest, assert_joinFromWrongThread)
{
    auto noCoreDumps = disableCoreDumps();
    Thread thread;
    thread.post([&] {
        notifyStarted();
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
        ASSERT_DEATH({ thread.join([] {}); }, "");
    });
    waitUntilStarted();
    thread.syncJoin();
}
