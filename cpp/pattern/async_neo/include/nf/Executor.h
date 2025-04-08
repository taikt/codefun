/*
 * BMW Neo Framework
 *
 * Copyright (C) 2018-2023 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 *     Marko Popović <marko.popovic@bmw.de>
 *     Florian Franzmann <Florian.Franzmann@ul.com>
 *     Szymon Wojtczak <szymon.wojtczak@globallogic.com>
 */

#pragma once

#include <nf/Function.h>
#include <nf/LinuxSignal.h>

#include <chrono>
#include <memory>
#include <thread>

NF_BEGIN_NAMESPACE

class Timer;
class IoWatch;

/**
 * @ingroup nf_core_Executors
 * @brief The executor.
 *
 * An executor is an interface towards an underlying event loop. It allows to enqueue an
 * executable task, e.g. a function or a lambda, to be executed in a context of an associated
 * thread.
 *
 * @par Thread-safety
 *
 * All methods except @ref run() are thread-safe.
 */
class Executor
{
public:
    using Task = Function<void()>;
    using Delay = std::chrono::milliseconds;
    using TimerTask = Function<bool()>;
    using Interval = std::chrono::milliseconds;
    /// @copydoc nf::IoWatch::Task
    using IoWatchTask = Function<bool(std::int16_t)>;
    using ExceptionHandler = Function<void(const std::exception_ptr &)>;

public:
    /**
     * @brief Get an executor for a caller's thread.
     *
     * This method returns a default executor associated with a caller's thread. If it does
     * not exist, this method will create it.
     */
    static const std::shared_ptr<Executor> &thisThread() noexcept;

    /**
     * @brief Get an executor for the main thread.
     *
     * This method returns a default executor associated with the main thread, i.e. a thread
     * where the framework was @ref Framework::initialize() "initialized". This is the thread
     * of @ref BaseApplication (if this class is used).
     */
    static const std::shared_ptr<Executor> &mainThread() noexcept;

public:
    /**
     * @brief Post a task to the executor.
     *
     * This method enqueues a given task to be executed later (as soon as possible) by
     * this executor. It will be executed in a context of the executor's thread.
     *
     * @see @ref nf::post(Executor::Task&&)
     */
    virtual void post(Task &&task) noexcept = 0;

    /**
     * @brief Post a delayed task to the executor.
     *
     * This method enqueues a given task to be executed later (with a given @p delay) by
     * this executor. It will be executed in a context of the executor's thread.
     * If the @p delay is negative then it is truncated to zero.
     *
     * @see @ref nf::post(Executor::Delay,Executor::Task&&)
     */
    void post(Delay delay, Task &&task) noexcept;

    /**
     * @brief Make a timer using this executor.
     *
     * This method constructs a timer in a context of this executor's thread.
     * If the @p interval is negative then it is truncated to zero.
     */
    virtual std::unique_ptr<Timer> makeTimer(Interval interval, TimerTask &&task) noexcept = 0;

    /**
     * @brief Make an I/O watch using this executor.
     *
     * This method constructs an I/O watch in a context of this executor's thread.
     */
    virtual std::unique_ptr<IoWatch> makeIoWatch(int fd, std::int16_t events,
                                                 IoWatchTask &&task) noexcept = 0;

    /**
     * @brief Sets the vector of Linux signal handlers for this application.
     *
     * This method accepts vector of LinuxSignalHandler consisting of Linux signal code and the
     * corresponding handler to be executed for the signal.
     *
     * @param signalHandlers A vector of Linux signal handlers to be registered.
     *
     * @returns true if handler registration was successful, false otherwise
     *
     * @note Calling this method a second time will overwrite all currently registered signals
     * and their handlers.
     *
     * @note Passing an empty vector will restore default Linux signal handling.
     *
     * @see @ref nf_core_LinuxSignals
     */

    virtual bool setSignalHandlers(std::vector<LinuxSignalHandler> signalHandlers) noexcept = 0;

    /**
     * @brief Extends the vector of currently registered Linux signal handlers.
     *
     * @param signalHandlers A vector of signal handlers to be added to the vector
     * of handlers which are already registered.
     *
     * @return true if handler registration was successful, false otherwise
     *
     * @see @ref nf_core_LinuxSignals
     */
    virtual bool appendSignalHandlers(std::vector<LinuxSignalHandler> signalHandlers) noexcept = 0;

    /**
     * @brief Provides deleteLater() functionality for shared pointers.
     *
     * Immediately makes @p ptr a nullptr and deletes the object behind it when control
     * returns to the thread's event loop.
     */
    template <typename T>
    void deleteLater(std::shared_ptr<T> &ptr) noexcept
    {
        post([cntIncreaser = std::move(ptr)] {});
        ptr.reset();
    }

    /**
     * @brief Provides deleteLater() functionality for unique pointers.
     *
     * Immediately makes @p ptr a nullptr and deletes the object behind it when control
     * returns to the thread's event loop.
     */
    template <typename T>
    void deleteLater(std::unique_ptr<T> &ptr) noexcept
    {
        post([ptr = std::move(ptr)] {});
        ptr.reset();
    }

    /**
     * @brief Run the event loop.
     *
     * This method runs an underlying event loop and starts event processing. It does not
     * return until the event processing is @ref stop() "stopped".
     *
     * Calling this method if the event loop is already running is a no-op.
     *
     * See @ref nf_core_ExecutionFlow for documentation about exception handling.
     *
     * @return Executor's exit code
     *
     * @threadunsafe
     */
    int run();

    /**
     * @brief Stop the event loop.
     *
     * This method stops event processing after currently executed task.
     * Stopping an event loop which is not running is a no-op.
     *
     * @warning Do not invoke this method on the main thread's executor if @nfref{BaseApplication}
     *          is used. Doing so would skip several steps of the termination sequence required for
     *          a clean shutdown.
     *
     * @see @ref nf_exec_ExecutionModel_StartStop "Execution Model: Starting and Stopping Executors"
     * @see @ref nf_core_BaseApp_Shutdown "Base Application: Shutdown"
     */
    virtual void stop() noexcept = 0;
    /**
     * @brief Stop the event loop.
     *
     * This method stops event processing after currently executed task.
     * Stopping an event loop which is not running is a no-op.
     *
     * @warning Do not invoke this method on the main thread's executor if @nfref{BaseApplication}
     *          is used. Doing so would skip several steps of the termination sequence required for
     *          a clean shutdown.
     *
     * @param exitCode Executor's exit code
     *
     * @see @ref nf_exec_ExecutionModel_StartStop "Execution Model: Starting and Stopping Executors"
     * @see @ref nf_core_BaseApp_Shutdown "Base Application: Shutdown"
     */
    virtual void stop(int exitCode) noexcept = 0;

    /**
     * @brief Get current time of the executor.
     *
     * Returns a value expressed in @c milliseconds being a time point, which a concrete executor
     * considers as the current time. This value is used to schedule tasks using formula
     * `now() + delay`.
     *
     * @note A client *must not* interpret the returned value as system time.
     * @see nf::ISystemClock.
     *
     * @since 4.14
     */
    virtual std::chrono::milliseconds now() const noexcept = 0;

    /**
     * @brief Register an exception handler with the @c Executor.
     *
     * The exception handler registered via this method will be called when a task throws an
     * exception. This method must be called before the call to @c run().
     *
     * The default exception handler just re-throws the exception.
     *
     * In case the exception handler itself throws, the event loop is stopped,
     * all posted events are discarded and @c run() re-throws that exception.
     *
     * Otherwise the exception is considered handled and execution of the event
     * loop resumes.
     *
     * @since 4.17
     *
     */
    void setExceptionHandler(ExceptionHandler handler);

    /**
     * @brief Check if this thread is the same thread of the @c Executor.
     *
     * The method checks if it is being called in the same thread as the the executor which it
     * belongs to. If the executor is not running the method will always return false. The check is
     * based on thread identifier.
     *
     * @return true if if this thread is the same thread of the @c Executor, false otherwise or when
     * the executor is stopped.
     *
     * @since 5.0
     *
     */
    bool isThisThread() const noexcept;

protected:
    Executor() noexcept;
    virtual ~Executor() = default;

private:
    /**
     * @brief Run the event loop.
     *
     * This method must be implemented as a private method by derived classes.
     *
     * @c run() handles exceptions, so implementations of runImpl() can let
     * exceptions thrown by event handlers escape.
     *
     * @return Executor's exit code
     *
     * See the documentation of @c run().
     */
    virtual int runImpl() = 0;

    /**
     * @brief Post a delayed task to the executor.
     *
     * This method must be implemented as a private method by derived classes.
     * It will be called by @c post(Delay, Task&&)
     *
     * @see See the documentation of @ref Executor::post(Delay, Task&&)
     */
    virtual void postImpl(Delay delay, Task &&task) noexcept = 0;

    /**
     * @brief Resets the Executor's queue.
     *
     * This method must be implemented as a private method by derived classes.
     *
     * Postcondition: All handlers have been removed from the event queue and
     * the Executor object is in a pristine state.
     *
     * It will be called by @c run() after the exception handler has thrown an
     * exception.
     *
     */
    virtual void discardAllPostedEvents() noexcept = 0;

protected:
    std::thread::id m_threadId;

private:
    ExceptionHandler m_exceptionHandler;
};

/**
 * @ingroup nf_core_Executors
 * @brief Post a task to the executor of the current thread.
 *
 * Equivalent of <code>@ref nf::Executor::thisThread() "nf::Executor::thisThread()"->@ref
 * nf::Executor::post(Task&&) "post(task)"</code>.
 */
inline void post(Executor::Task &&task) noexcept
{
    Executor::thisThread()->post(std::move(task));
}

/**
 * @ingroup nf_core_Executors
 * @brief Post a delayed task to the executor of the current thread. If the @p delay is negative
 * then it is truncated to zero.
 *
 * Equivalent of <code>@ref nf::Executor::thisThread() "nf::Executor::thisThread()"->@ref
 * nf::Executor::post(Delay,Task&&) "post(delay, task)"</code>.
 */
inline void post(Executor::Delay delay, Executor::Task &&task) noexcept
{
    Executor::thisThread()->post(delay, std::move(task));
}

NF_END_NAMESPACE
