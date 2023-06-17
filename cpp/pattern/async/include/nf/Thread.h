/*
 * BMW Neo Framework
 *
 * Copyright (C) 2020 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#pragma once

#include "Enums.h"
#include "Executor.h"

#include <boost/noncopyable.hpp>

#include <mutex>
#include <queue>
#include <thread>

NF_BEGIN_NAMESPACE

/**
 * @ingroup nf_core_Executors
 * @brief The thread.
 *
 * This class represents a single thread of execution with a running event loop (@ref Executor).
 * One can @ref post() tasks to be executed in a context of this new thread.
 *
 * A new thread is created together with a @c Thread object. After that, one can immediately
 * start @ref post() "posting" tasks. They will be executed in a context of the new thread
 * as soon as it becomes active and an event loop is created.
 *
 * Posted tasks are executed in the order they were posted. The tasks are not executed if the
 * executor for that thread is stopped.
 *
 * The thread will remain active and accept new tasks until it is @ref join() "joined".
 *
 * @par Thread-safety
 *
 * All public methods of a @c Thread object must be invoked only from the thread where this
 * object was created.
 *
 * @par Strict contract
 *
 * Multi-threaded programming is a source of errors. In order to ensure that this class is
 * used correctly, it implements a "terminate a program if the contract is violated" rule.
 *
 * @par Example
 *
 * One can use this class to execute some tasks in a worker thread, e.g.:
 * @code
 * nf::Thread m_thread;
 * ...
 * void doStuff()
 * {
 *     m_thread.post([] {
 *         // Invoked in a context of a worker thread
 *         auto result = doSomethingHeavy();
 *         nf::Executor::mainThread()->post([result] {
 *             // Invoked again in a context of the main thread
 *             process(result);
 *         });
 *     });
 * }
 * @endcode
 *
 * @since 3.11
 *
 * @see @ref nf_exec_ExecutionModel_StartStop "Execution Model: Starting and Stopping"
 *
 */
class Thread : boost::noncopyable
{
public:
    using Task = Executor::Task;

public:
    /**
     * @brief Create a thread.
     */
    Thread() noexcept;

    /**
     * @brief Destroy a thread.
     * @pre The thread must be fully joined. Otherwise the program will terminate.
     */
    ~Thread();

public:
    /**
     * @brief Post a task to this thread's executor.
     * @pre The thread must not be joined. Otherwise the program will terminate.
     * @see @ref Executor::post().
     */
    void post(Task &&task) noexcept;

    /**
     * @brief Join the thread.
     * @pre The thread must not be joined. Otherwise the program will terminate.
     *
     * This method will stop the thread's executor and join the running thread. This method
     * is asynchronous, it will return immediately and later invoke @e onJoined when the
     * thread is joined. The callback will be invoked in a context of the caller's thread.
     */
    void join(Task &&onJoined) noexcept;

    /**
     * @brief Join the thread (synchronously).
     * @pre The thread must not be joined. Otherwise the program will terminate.
     *
     * This method will stop the thread's executor and join the running thread. This method
     * will block until the thread is joined.
     */
    void syncJoin() noexcept;

private:
    NF_ENUM_CLASS(Status, Starting, Running, Joining, Joined)

private:
    void worker() noexcept;
    void finishJoin() noexcept;

private:
    std::mutex m_mutex;
    std::thread::id m_creatorId;
    Status m_status{Status::Starting};
    std::shared_ptr<Executor> m_executor;
    std::queue<Task> m_tasks;
    std::thread m_thread;
};

NF_END_NAMESPACE
