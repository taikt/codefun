/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include "nf/IoWatch.h"

#include <nf/Assert.h>
#include <nf/Executor.h>
#include <nf/Logging.h>
#include <nf/Printable.h>

#include <cstring>

using namespace nf;

std::unique_ptr<IoWatch> IoWatch::construct(int fd, std::int16_t events, Task &&task) noexcept
{
    return Executor::thisThread()->makeIoWatch(fd, events, std::move(task));
}

IoWatch::IoWatch(int fd, std::int16_t events, Task &&task) noexcept
    : m_task(std::move(task))
    , m_fd(fd)
    , m_events(events)
    , m_isActive(false)
{
    nf::verbose("I/O watch {} created with fd={} and events={}", fmt::ptr(this), fd, events);
}

IoWatch::~IoWatch()
{
    nf::verbose("I/O watch {} destroyed", fmt::ptr(this));
}

bool IoWatch::isActive() const noexcept
{
    return m_isActive;
}

bool IoWatch::isReadWatch() const noexcept
{
    return (m_events & (POLLIN | POLLRDNORM | POLLPRI | POLLRDBAND)) != 0;
}

bool IoWatch::isWriteWatch() const noexcept
{
    return (m_events & (POLLOUT | POLLWRNORM | POLLWRBAND)) != 0;
}

void IoWatch::notify() noexcept
{
    /* Sanity check that a type of m_events match a type accepted by poll(). This is needed
     * because we cannot use "short" as it is not safe and clang-tidy complains about it. */
    static_assert(std::is_same_v<decltype(IoWatch::m_events), decltype(::pollfd::events)>);

    ::pollfd pfd{m_fd, m_events, 0};
    int ret = ::poll(&pfd, 1, 0);

    nf::verbose("I/O watch {} poll results: {}; revents={}", fmt::ptr(this), ret, pfd.revents);

    if (ret > 0) {
        if (!m_task(pfd.revents)) {
            /* Do not restart the watch! */
            return;
        }
    } else if (ret < 0) {
        nf::fatal("Poll for fd={} failed with {}", m_fd, nf::strerror(errno));
        /* The current strategy is to restart the watch even if poll() fails. If we do not
         * do this, clients will never get a notification. If we do it, there is a chance
         * to end up in an infinite loop if poll() constantly fails. This, however, does not
         * happen in practice (as shown by e.g. NaRE). */
    }

    nf::verbose("Restarting an I/O watch {}", fmt::ptr(this));
    startWatch();
}

void IoWatch::start() noexcept
{
    if (m_isActive) {
        nf::verbose("I/O watch {} is already started!", fmt::ptr(this));
        return;
    }

    nf::verbose("Starting an I/O watch {}", fmt::ptr(this));
    startWatch();
}

void IoWatch::stop() noexcept
{
    if (m_isActive) {
        nf::verbose("Stopping an I/O watch {}", fmt::ptr(this));
        stopWatch();
    }
}

void IoWatch::startWatch() noexcept
{
    m_isActive = true;
    onStarted();
}

void IoWatch::stopWatch() noexcept
{
    m_isActive = false;
    onStopped();
}
