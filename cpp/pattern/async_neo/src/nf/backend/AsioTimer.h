/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019-2020 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#pragma once

#include <nf/Timer.h>
#include <nf/detail/CallbackScope.h>

#include <boost/asio/io_service.hpp>
#include <boost/asio/steady_timer.hpp>

NF_BEGIN_NAMESPACE

namespace backend {

class AsioTimer : public Timer
{
public:
    AsioTimer(boost::asio::io_service &ios, Interval interval, Task &&task) noexcept;
    void start() noexcept override;
    void stop() noexcept override;
    Interval remainingTime() const noexcept override;

    void handle(const boost::system::error_code &ec) noexcept;
    void setAutoDelete() noexcept;

private:
    boost::asio::steady_timer m_timer;
    detail::CallbackScope m_cbScope;
    bool m_isAutoDelete{false};
};

} // namespace backend

NF_END_NAMESPACE
