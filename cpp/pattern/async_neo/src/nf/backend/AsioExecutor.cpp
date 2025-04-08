/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 *     Florian Franzmann <Florian.Franzmann@ul.com>
 */

#include "nf/backend/AsioExecutor.h"

#include "AsioIoWatch.h"
#include "AsioTimer.h"

#include <nf/Logging.h>

#include <boost/version.hpp>

/*
 * TODO(NODE0DEV-213): Many of the methods of io_service have been deprecated
 * and io_service has been renamed to io_context. Once we've switched to a more
 * modern version of ASIO we should refactor our code to use their more modern
 * equivalents.
 *
 * cf. https://www.boost.org/doc/libs/1_66_0/doc/html/boost_asio/reference/io_service.html
 */

using namespace nf;
using namespace backend;
using namespace detail;

namespace {

struct TaskWrapper
{
    Executor::Task task;
    explicit TaskWrapper(Executor::Task &&task) noexcept;
    TaskWrapper(const TaskWrapper &other) noexcept;
    void operator()() const;
};

TaskWrapper::TaskWrapper(Executor::Task &&task) noexcept
    : task(std::move(task))
{
}

TaskWrapper::TaskWrapper(const TaskWrapper &other) noexcept
    : task(std::move(const_cast<TaskWrapper &>(other).task)) // NOLINT
{
}

void TaskWrapper::operator()() const
{
    task();
}

} // anonymous namespace

AsioExecutor::AsioExecutor() = default;

AsioExecutor::~AsioExecutor() = default;

void AsioExecutor::post(Task &&task) noexcept
{
    m_ios.post(TaskWrapper(std::move(task)));
}

void AsioExecutor::postImpl(Delay delay, Task &&task) noexcept
{
    std::unique_lock lock(m_mutex);
    auto timer = m_postTimers.insert(m_postTimers.end(), nullptr);
    *timer = std::make_unique<AsioTimer>(m_ios, delay, [this, timer, task = std::move(task)] {
        task();

        /* We cannot delete the timer object right now because it is being accessed after
         * after this handle returns. So we tell it to delete itself when it is safe to do
         * so and non-destructively remove the pointer from the list. */
        timer->release()->setAutoDelete();
        m_postTimers.erase(timer);
        return false;
    });
    (*timer)->start();
}

std::unique_ptr<Timer> AsioExecutor::makeTimer(Interval interval, TimerTask &&task) noexcept
{
    return std::make_unique<AsioTimer>(m_ios, interval, std::move(task));
}

std::unique_ptr<IoWatch> AsioExecutor::makeIoWatch(int fd, std::int16_t events,
                                                   IoWatchTask &&task) noexcept
{
    return std::make_unique<backend::AsioIoWatch>(m_ios, fd, events, std::move(task));
}

int AsioExecutor::runImpl()
{
    m_work = std::make_unique<boost::asio::io_service::work>(m_ios);
    m_exitCode = EXIT_SUCCESS;
    boost::system::error_code ec;

    nf::info("Running an event loop {}", fmt::ptr(this));
    m_ios.run(ec);
    m_ios.reset();
    nf::info("Event loop {} finished with {}", fmt::ptr(this), ec.message());

    return m_exitCode;
}

void AsioExecutor::stop() noexcept
{
    stop(EXIT_SUCCESS);
}

void AsioExecutor::stop(int exitCode) noexcept
{
    nf::info("Stopping an event loop {}", fmt::ptr(this));
    m_ios.stop();
    m_work.reset();
    m_cbScope.reset();
    m_asioSignals.reset();
    m_exitCode = exitCode;
}

boost::asio::io_service &AsioExecutor::ioService()
{
    return m_ios;
}

bool AsioExecutor::setSignalHandlers(std::vector<LinuxSignalHandler> signalHandlers) noexcept
{
    if (!m_asioSignals) {
        m_asioSignals = std::make_unique<boost::asio::signal_set>(m_ios);
    }

    boost::system::error_code ec;
    m_asioSignals->clear(ec);
    if (ec) {
        nf::error("Can not clear signal vector. Error: {}", ec.message());
        return false;
    }

    if (signalHandlers.empty()) {
        nf::info("Cleared Linux signal handling");
        m_signalHandlers.clear();

        return true;
    }

    m_cbScope.reset();

    ec.clear();
    for (const auto &handle : signalHandlers) {
        m_asioSignals->add(static_cast<int>(handle.signal), ec);
        if (ec) {
            nf::error("Can not add new signal. Error: {}", ec.message());
            return false;
        }
    }

    m_signalHandlers = std::move(signalHandlers);

    m_asioSignals->async_wait(m_cbScope.bind(
        [this](const auto &ec, int signalNumber) { handleSignals(ec, signalNumber); }));

    return true;
}

bool AsioExecutor::appendSignalHandlers(std::vector<LinuxSignalHandler> signalHandlers) noexcept
{
    signalHandlers.insert(std::end(signalHandlers), std::begin(m_signalHandlers),
                          std::end(m_signalHandlers));
    return setSignalHandlers(signalHandlers);
}

void AsioExecutor::handleSignals(const boost::system::error_code &ec, int signalNumber)
{
    if (ec) {
        nf::error("Caught signal {} with error code {}", signalNumber, ec);
        return;
    }

    auto linuxSignal = static_cast<LinuxSignal>(signalNumber);

    if (!isValid(linuxSignal)) {
        nf::error("Caught invalid signal {}", signalNumber);
        return;
    }

    nf::info("Caught linux signal {}", toString(linuxSignal));

    for (const auto &handle : m_signalHandlers) {
        if (linuxSignal == handle.signal) {
            handle.cb(handle.signal);
        }
    }
    /* Check if stop() has not been called before signal arrived. */
    if (m_asioSignals) {
        /* Re-enable linux signal for handling. */
        m_asioSignals->async_wait(m_cbScope.bind(
            [this](const auto &ec, int signalNumber) { handleSignals(ec, signalNumber); }));
    }
}

std::chrono::milliseconds AsioExecutor::now() const noexcept
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch());
}

void AsioExecutor::discardAllPostedEvents() noexcept
{
    /*
     * Destroying and recreating the io_service object seems to be the only way to flush
     * pending tasks. The io_service API has changed in boost 1.66; the changes
     * are intended to be backward compatible but in subtle ways they are not:
     */
#if BOOST_VERSION < 106600
    m_ios.~io_service();
#else
    m_ios.~io_context();
#endif
    new (&m_ios) boost::asio::io_service{};
}
