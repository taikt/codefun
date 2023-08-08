/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019-2021 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#pragma once

#include <nf/Function.h>

#include <boost/core/noncopyable.hpp>

#include <cstdint>
#include <memory>

#include <sys/poll.h>

NF_BEGIN_NAMESPACE

/**
 * @ingroup nf_core_Executors
 * @brief The I/O watch.
 *
 * This class represents an instance of an I/O watch. It's purpose is to watch for some events
 * on a file descriptor in an event loop aware manner.
 *
 * @par Example
 *
 * This code will notify the event loop (of the current thread) when a file descriptor has
 * data to be read.
 *
 * @code
 * #include <nf/IoWatch.h>
 *
 * m_watch = nf::IoWatch::construct(m_fd, POLLIN, [](std::int16_t events) {
 *     if ((events & POLLIN) != 0) {
 *         process(m_fd);
 *     }
 *     return false;
 * });
 * m_watch->start();
 * @endcode
 *
 * @par Thread-safety
 *
 * This class is not thread safe. It must be accessed only from a thread it has been created
 * for.
 */
class IoWatch : private boost::noncopyable
{
public:
    /**
     * @brief The I/O watch task.
     *
     * This is a type of an action executed by a watch when an event of a watched type occurs.
     * A task must not throw exceptions. It must not delete a watch which invokes this task.
     *
     * The task must accept an @c int16_t (a.k.a. @c short). It contains a bit mask of occurred
     * events. See a manual for <a href="http://man7.org/linux/man-pages/man2/poll.2.html">
     * poll()</a> for what the bits mean.
     *
     * The task must return a boolean. If it returns @e true, the watch will remain active.
     * Note, that in this case the task must consume the I/O event, otherwise the task will
     * be again immediately posted to the event loop. This might result in a busy looping.
     * If it returns @e false, the watch will be stopped.
     */
    using Task = Function<bool(std::int16_t)>;

public:
    /**
     * @brief Construct an I/O watch.
     * @param fd File descriptor to watch.
     * @param events Bit mask of events to watch for.
     * @param task I/O watch task.
     *
     * This function constructs an I/O watch with the given @p fd, @p events and @p task.
     * The watch is created in a context of the current thread. This is the same as @ref
     * Executor::makeIoWatch() "Executor::thisThread()->makeIoWatch()".
     */
    static std::unique_ptr<IoWatch> construct(int fd, std::int16_t events, Task &&task) noexcept;

public:
    virtual ~IoWatch();

    /**
     * @brief Check if the I/O watch is active.
     */
    bool isActive() const noexcept;

    /**
     * @brief Start an I/O watch.
     *
     * This method starts an I/O watch. If the watch is already started, this is a no-op.
     */
    void start() noexcept;

    /**
     * @brief Stop an active I/O watch.
     *
     * This method stops an active I/O watch. If the watch is already stopped, it is a no-op.
     */
    void stop() noexcept;

protected:
    IoWatch(int fd, std::int16_t events, Task &&task) noexcept;
    bool isReadWatch() const noexcept;
    bool isWriteWatch() const noexcept;
    void startWatch() noexcept;
    void stopWatch() noexcept;
    void notify() noexcept;

private:
    virtual void onStarted() noexcept = 0;
    virtual void onStopped() noexcept = 0;

protected:
    Task m_task;
    int m_fd;
    std::int16_t m_events;

private:
    bool m_isActive;
};

NF_END_NAMESPACE
