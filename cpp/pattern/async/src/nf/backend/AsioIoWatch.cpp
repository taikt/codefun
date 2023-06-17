/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include "AsioIoWatch.h"

#include <nf/Logging.h>

using namespace nf::backend;

AsioIoWatch::AsioIoWatch(boost::asio::io_service &ios, int fd, std::int16_t events,
                         Task &&task) noexcept
    : IoWatch(fd, events, std::move(task))
    , m_watch(ios, fd)
{
}

AsioIoWatch::~AsioIoWatch() = default;

void AsioIoWatch::onStarted() noexcept
{
    /// @todo Use async_wait when boost is upgraded.

    try {
        if (isReadWatch()) {
            m_watch.async_read_some(boost::asio::null_buffers(),
                                    m_cbScope.bind([this](const auto &ec, auto) { handle(ec); }));
        }
        if (isWriteWatch()) {
            m_watch.async_write_some(boost::asio::null_buffers(),
                                     m_cbScope.bind([this](const auto &ec, auto) { handle(ec); }));
        }
    } catch (const std::exception &ex) {
        nf::fatal("Failed to start an I/O watch: {}", ex.what());
        stop();
    }
}

void AsioIoWatch::onStopped() noexcept
{
    try {
        m_watch.cancel();
    } catch (const std::exception &ex) {
        nf::fatal("Failed to stop an I/O watch: {}", ex.what());
    }
}

void AsioIoWatch::handle(const boost::system::error_code &ec) noexcept
{
    nf::verbose("I/O watch {} finished with {}", fmt::ptr(this), ec);

    stopWatch();

    if (!ec) {
        notify();
    }
}
