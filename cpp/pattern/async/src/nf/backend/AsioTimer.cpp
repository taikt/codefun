/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include "AsioTimer.h"

#include <nf/ChronoIoStream.h>
#include <nf/Logging.h>

using namespace nf::backend;

AsioTimer::AsioTimer(boost::asio::io_service &ios, Interval interval, Task &&task) noexcept
    : Timer(interval, std::move(task))
    , m_timer(ios)
{
    nf::verbose("Timer {} created with an interval of {}", fmt::ptr(this), interval);
}

void AsioTimer::start() noexcept
{
    if (isRunning()) {
        nf::verbose("Restarting a timer {}", fmt::ptr(this));
        stop();
    }

    nf::verbose("Starting a timer {}", fmt::ptr(this));

    /* If the timer is started inside a timer-task, then do nothing. The timer will be
     * rescheduled as soon the control returns to handle(). */
    if (isInTask()) {
        m_state = State::InTaskStarted;
        return;
    }

    m_state = State::Started;

    try {
        m_timer.expires_from_now(m_interval);
        m_timer.async_wait(m_cbScope.bind([this](const auto &ec) { handle(ec); }));
    } catch (const std::exception &ex) {
        nf::fatal("Failed to start a timer: {}", ex.what());
    }
}

void AsioTimer::stop() noexcept
{
    if (!isRunning()) {
        return;
    }

    nf::verbose("Stopping a timer {}", fmt::ptr(this));

    /* If the timer is stopped inside a timer task, then do nothing. The timer will not
     * be rescheduled when the control returns to handle(). */
    if (isInTask()) {
        m_state = State::InTaskStopped;
        return;
    }

    m_state = State::Stopped;

    /* Do not invoke handle() any more. */
    m_cbScope.reset();

    try {
        m_timer.cancel();
    } catch (const std::exception &ex) {
        nf::error("Failed to stop a timer: {}", ex.what());
    }
}

AsioTimer::Interval AsioTimer::remainingTime() const noexcept
{
    const auto zero = Interval::zero();
    const auto remainingTime =
        std::chrono::duration_cast<std::chrono::milliseconds>(m_timer.expires_from_now());

    return (m_state == State::Stopped) ? zero : std::max(zero, remainingTime);
}

void AsioTimer::handle(const boost::system::error_code &ec) noexcept
{
    nf::verbose("Timer {} triggered with ec={}", fmt::ptr(this), ec);

    if (ec != boost::asio::error::operation_aborted) {
        m_state = State::InTaskNeutral;
        const bool doContinue = m_task();

        /* Timer must be rescheduled if:
         *  - task explicitly starts the timer (regardless of what it returns)
         *  - task returns true AND does not explicitly stop the timer. */
        if (m_state == State::InTaskStarted || (doContinue && m_state == State::InTaskNeutral)) {
            nf::verbose("Rescheduling a timer {}", fmt::ptr(this));
            m_state = State::Started;
            try {
                m_timer.expires_from_now(m_interval);
                m_timer.async_wait(m_cbScope.bind([this](const auto &ec) { handle(ec); }));
            } catch (const std::exception &ex) {
                nf::fatal("Failed to reschedule a timer: {}", ex.what());
            }
            return;
        }
    }

    nf::verbose("Timer {} has been stopped", fmt::ptr(this));
    m_cbScope.reset();
    m_state = State::Stopped;
    if (m_isAutoDelete) {
        delete this;
    }
}

void AsioTimer::setAutoDelete() noexcept
{
    m_isAutoDelete = true;
}
