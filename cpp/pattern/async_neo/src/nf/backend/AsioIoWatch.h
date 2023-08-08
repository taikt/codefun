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

#include <nf/IoWatch.h>
#include <nf/detail/CallbackScope.h>

#include <boost/asio/io_service.hpp>
#include <boost/asio/posix/stream_descriptor.hpp>

NF_BEGIN_NAMESPACE

namespace backend {

class AsioIoWatch final : public IoWatch
{
public:
    AsioIoWatch(boost::asio::io_service &ios, int fd, std::int16_t events, Task &&task) noexcept;
    ~AsioIoWatch() override;

private:
    void onStarted() noexcept override;
    void onStopped() noexcept override;
    void handle(const boost::system::error_code &ec) noexcept;

private:
    boost::asio::posix::stream_descriptor m_watch;
    detail::CallbackScope m_cbScope;
};

} // namespace backend

NF_END_NAMESPACE
