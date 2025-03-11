/*
 * BMW Neo Framework
 *
 * Copyright (C) 2020-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include "nf/Thread.h"

#include <nf/Assert.h>
#include <nf/ContainerAlgorithms.h>
#include <nf/Logging.h>

using namespace nf;

Thread::Thread() noexcept
    : m_creatorId(std::this_thread::get_id())
    , m_thread([this] { worker(); })
{
    nf::info("Starting a thread with id {}", m_thread.get_id());
}

Thread::~Thread()
{
    assertThat(m_creatorId == std::this_thread::get_id(),
               "Only a thread which created nf::Thread can destroy it");

    std::unique_lock lock(m_mutex);
    assertThat(m_status == Status::Joined, "Cannot destroy a non-joined thread");
}

void Thread::post(Task &&task) noexcept
{
    assertThat(m_creatorId == std::this_thread::get_id(),
               "Only a thread which created nf::Thread can post tasks");

    std::unique_lock lock(m_mutex);

    switch (m_status) {
    case Status::Starting:
        /* At this point the executor is not created yet. */
        m_tasks.push(std::move(task));
        break;
    case Status::Running:
        m_executor->post(std::move(task));
        break;
    case Status::Joining:
    case Status::Joined:
        fail("Cannot post a task to a joined thread");
    }
}

void Thread::join(Task &&onJoined) noexcept
{
    assertThat(m_creatorId == std::this_thread::get_id(),
               "Only a thread which created nf::Thread can join it");

    nf::info("Joining a thread {}", m_thread.get_id());

    /* Join the thread in the caller's thread. */
    auto notifyJoined = [this, executor = Executor::thisThread(),
                         onJoined = std::move(onJoined)]() mutable {
        executor->post([this, onJoined = std::move(onJoined)]() mutable {
            finishJoin();
            onJoined();
        });
    };

    {
        std::unique_lock lock(m_mutex);
        switch (m_status) {
        case Status::Starting:
            /* The thread's event loop is not running yet and it will not be started any
             * more, so we can just join the thread. */
            m_status = Status::Joining;
            notifyJoined();
            return;
        case Status::Running:
            /* We need to stop the thread's event loop. The implementation is below. */
            m_status = Status::Joining;
            break;
        case Status::Joining:
        case Status::Joined:
            fail("Cannot join an already joined thread");
        }
    }

    /* We need to stop the thread's executor. Doing this from the thread itself by using
     * post() we ensure that the thread is running and the executor exists when we are
     * calling stop(). */
    m_executor->post([this, notifyJoined = std::move(notifyJoined)]() mutable {
        m_executor->stop();

        /* Now, when the thread's event loop is not running, the thread will terminate soon
         * after we exit this handler. Now we can join() the thread, but we can do it only
         * in the caller's thread. This will not block the caller's thread because at the
         * point when this code is executed, the thread should be already terminated or
         * (less likely) is about to terminate. */
        notifyJoined();
    });
}

void Thread::syncJoin() noexcept
{
    assertThat(m_creatorId == std::this_thread::get_id(),
               "Only a thread which created nf::Thread can join it");

    nf::info("Joining (sync) a thread {}", m_thread.get_id());

    {
        std::unique_lock lock(m_mutex);
        switch (m_status) {
        case Status::Starting:
            /* The thread's event loop is not running yet and it will not be started any
             * more, so we can just join the thread. */
            m_status = Status::Joining;
            break;
        case Status::Running:
            /* We need to stop the thread's event loop. */
            m_status = Status::Joining;
            m_executor->post([this] { m_executor->stop(); });
            break;
        case Status::Joining:
        case Status::Joined:
            fail("Cannot join an already joined thread");
        }
    }

    finishJoin();
}

void Thread::worker() noexcept
{
    nf::info("Thread {} has been started!", m_thread.get_id());

    {
        std::unique_lock lock(m_mutex);

        /* Do not create and run an executor if the thread is already being joined. */
        if (m_status == Status::Joining) {
            return;
        }

        m_executor = Executor::thisThread();
        m_status = Status::Running;

        while (!m_tasks.empty()) {
            m_executor->post(nf::pop(m_tasks));
        }
    }

    try {
        m_executor->run();
    } catch (const std::exception &e) {
        nf::error("The executor of thread {} has thrown ({}), terminating.", m_thread.get_id(),
                  e.what());

        std::terminate();
    } catch (...) {
        nf::error("The executor of thread {} has thrown, terminating.", m_thread.get_id());

        std::terminate();
    }
}

void Thread::finishJoin() noexcept
{
    m_thread.join();
    nf::info("Thread {} has been joined!", m_thread.get_id());
    m_status = Status::Joined;
}
