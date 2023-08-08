/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#pragma once

#include <nf/Function.h>

#include <boost/core/noncopyable.hpp>

#include <chrono>
#include <memory>


NF_BEGIN_NAMESPACE

/**
 * @ingroup nf_core_Executors
 * @brief The timer.
 *
 * @code
 * #include <nf/Timer.h>
 * @endcode
 *
 * This class provides an interface to create and control timers. A timer is defined by a
 * @ref Task it needs to execute and an interval at which to execute this task.
 *
 * @par Periodic Timers
 *
 * Periodic timers execute the task at constant intervals. To construct a periodic timer the
 * task needs to return @e true.
 *
 * This code will execute @c doSomething() each ten seconds until the timer is explicitly
 * stopped.
 *
 * @code
 * auto timer = nf::Timer::construct(10s, [] {
 *     doSomething();
 *     return true;
 * });
 * timer->start();
 * @endcode
 *
 * @par Single-shot Timers
 *
 * Single-shot timers execute the task only once after the specified interval. To construct
 * a single-shot timer the task needs to return @e false.
 *
 * This code will execute @c doSomething() only once after ten seconds.
 *
 * @code
 * auto timer = nf::Timer::construct(10s, [] {
 *     doSomething();
 *     return false;
 * });
 * timer->start();
 * @endcode
 *
 * An alternative to single-shot timers is @ref Executor::post(Delay,Task&&).
 *
 * @par Thread Affinity and Safety
 *
 * When created, the timer is bound to a specific @ref Executor and its thread. The task will
 * be executed in a context of this thread. The @ref construct() method constructs a timer
 * for the current thread. Executor's @nfrefmethod{Executor,makeTimer()} constructs a timer
 * for that executor.
 *
 * This class is @b NOT thread safe. It must be accessed only from the thread it has been
 * created for.
 */
class Timer : private boost::noncopyable
{
public:
    /**
     * @brief The timer task.
     *
     * This is a type of an action executed by a timer when it expires. A task must not throw
     * exceptions. It must not delete a timer which invokes this task.
     *
     * The task must return a boolean. If it returns @e true, the timer will be restarted
     * with the same interval. Use it for periodic timers. If it returns @e false, the timer
     * will not be restarted. Use it for single-shot timers.
     */
    using Task = Function<bool()>;
    using Interval = std::chrono::milliseconds;

public:
    /**
     * @brief Construct a timer.
     * @param interval Timer interval.
     * @param task Timer task.
     *
     * This function constructs a timer with the given @p interval and @p task. The timer is
     * created in a context of the current thread. If the @p interval is negative
     * then it is truncated to zero. This is the same as @ref Executor::makeTimer()
     * "Executor::thisThread()->makeTimer()".
     */
    static std::unique_ptr<Timer> construct(Interval interval, Task &&task) noexcept;

public:
    virtual ~Timer();

    /**
     * @brief Set a timer interval.
     *
     * An interval defines a time how often a provided task is invoked. The negative value of @p
     * interval will have the same effect as using @c 0 - the task will be invoked as soon as
     * possible after previous invocation.
     *
     * @note This method must not be called on a running timer.
     */
    void setInterval(Interval interval) noexcept;

    /**
     * @brief Set a timer task.
     *
     * A task defines an action to execute when a provided interval passes.
     *
     * @note This method must not be called on a running timer.
     */
    void setTask(Task &&task) noexcept;

    /**
     * @brief Check if the timer is running.
     */
    bool isRunning() const noexcept;

    /**
     * @brief Start a timer.
     *
     * This method starts a timer with a previously set interval and a task. If the timer
     * is already running, it is restarted.
     *
     * @par Starting Timers in Tasks
     *
     * If a timer is explicitly started from its own task, the timer will be restarted after
     * the task finishes regardless of the task's return value.
     *
     * This code effectively makes a periodic timer despite returning @e false.
     *
     * @code
     * m_timer = nf::Timer::construct(10s, [this] {
     *     m_timer->start();
     *     return false;
     * });
     * @endcode
     */
    virtual void start() noexcept = 0;

    /**
     * @brief Stop a running timer.
     *
     * This method stops a timer. If the timer is already stopped, it is a no-op.
     *
     * @par Stopping Timers in Tasks
     *
     * If a timer is explicitly stopped from its own task, the timer will be stopped after
     * the task finishes regardless of the task's return value.
     *
     * This code effectively makes a single-shot timer despite returning @e true.
     *
     * @code
     * m_timer = nf::Timer::construct(10s, [this] {
     *     m_timer->stop();
     *     return true;
     * });
     * @endcode
     */
    virtual void stop() noexcept = 0;

    /**
     * @brief Get a time until the timer is expired.
     *
     * If the timer is already expired or has never been started, this method will return
     * zero.
     */
    virtual Interval remainingTime() const noexcept = 0;

protected:
    Timer(Interval interval, Task &&task) noexcept;
    bool isInTask() const noexcept;

private:
    Interval sanitizeInterval(Interval interval) const noexcept;

protected:
    enum class State
    {
        Stopped,
        Started,
        InTaskNeutral,
        InTaskStopped,
        InTaskStarted
    };

protected:
    Task m_task;
    Interval m_interval;
    State m_state;
};

NF_END_NAMESPACE
