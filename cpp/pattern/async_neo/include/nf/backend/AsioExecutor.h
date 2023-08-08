/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 *     Alexey Larikov <alexey.larikov@bmw.de>
 */

#pragma once

#include <nf/Executor.h>
#include <nf/detail/CallbackScope.h>

#include <boost/asio/io_service.hpp>
#include <boost/asio/signal_set.hpp>

#include <list>
#include <mutex>

NF_BEGIN_NAMESPACE

namespace backend {

class AsioTimer;

/**
 * @ingroup nf_Core
 * @brief The NF boost::asio executor implementation.
 *
 * This executor is based on @c boost::asio::io_service object. See boost::asio <a href=
 * "https://www.boost.org/doc/libs/1_65_0/doc/html/boost_asio.html"> documentation</a> for
 * more details.
 */
class AsioExecutor final : public Executor
{
public:
    AsioExecutor();
    ~AsioExecutor() override;
    void post(Task &&task) noexcept override;
    std::unique_ptr<Timer> makeTimer(Interval interval, TimerTask &&task) noexcept override;
    std::unique_ptr<IoWatch> makeIoWatch(int fd, std::int16_t events,
                                         IoWatchTask &&task) noexcept override;
    void stop() noexcept override;
    void stop(int exitCode) noexcept override;
    bool setSignalHandlers(std::vector<LinuxSignalHandler> signalHandlers) noexcept override;
    bool appendSignalHandlers(std::vector<LinuxSignalHandler> signalHandlers) noexcept override;

    /**
     * @brief Get current time of the executor.
     *
     * Returns @c steady_time as @c boost::asio::steady_timer is used internally.
     */
    std::chrono::milliseconds now() const noexcept override;

    /**
     * @brief Get underlying ASIO service
     */
    boost::asio::io_service &ioService();

private:
    void handleSignals(const boost::system::error_code &ec, int signalNumber);
    int runImpl() override;
    void postImpl(Delay delay, Task &&task) noexcept override;
    void discardAllPostedEvents() noexcept override;

private:
    boost::asio::io_service m_ios;
    std::unique_ptr<boost::asio::io_service::work> m_work;
    std::unique_ptr<boost::asio::signal_set> m_asioSignals;
    detail::CallbackScope m_cbScope;
    std::vector<LinuxSignalHandler> m_signalHandlers;
    std::list<std::unique_ptr<AsioTimer>> m_postTimers;
    std::mutex m_mutex;
    int m_exitCode{EXIT_SUCCESS};
};

} // namespace backend

NF_END_NAMESPACE
