/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include "nf/Timer.h"

#include <nf/Assert.h>
#include <nf/Executor.h>

using namespace nf;

std::unique_ptr<Timer> Timer::construct(Interval interval, Task &&task) noexcept
{
    return Executor::thisThread()->makeTimer(interval, std::move(task));
}

Timer::~Timer()
{
    assertThat(!isInTask(), "Timer must not be deleted from its own task");
}

void Timer::setInterval(Interval interval) noexcept
{
    if (!expectThat(!isRunning(), "Timer is not stopped")) {
        return;
    }
    m_interval = sanitizeInterval(interval);
}

void Timer::setTask(Task &&task) noexcept
{
    if (!expectThat(!isRunning(), "Timer is not stopped")) {
        return;
    }
    m_task = std::move(task);
}

bool Timer::isRunning() const noexcept
{
    return m_state != State::Stopped && m_state != State::InTaskStopped;
}

Timer::Timer(Interval interval, Task &&task) noexcept
    : m_task(std::move(task))
    , m_interval(sanitizeInterval(interval))
    , m_state(State::Stopped)
{
}

bool Timer::isInTask() const noexcept
{
    return m_state == State::InTaskNeutral || m_state == State::InTaskStarted
        || m_state == State::InTaskStopped;
}

Timer::Interval Timer::sanitizeInterval(Interval interval) const noexcept
{
    if (constexpr auto zero = std::chrono::milliseconds(0); interval < zero) {
        nf::warn("Truncating negative interval to 0");
        return zero;
    }
    return interval;
}
