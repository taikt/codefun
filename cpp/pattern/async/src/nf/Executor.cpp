/*
 * BMW Neo Framework
 *
 * Copyright (C) 2018-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 *     Marko Popović <marko.popovic@bmw.de>
 *     Florian Franzmann <Florian.Franzmann@ul.com>
 *     Szymon Wojtczak <szymon.wojtczak@globallogic.com>
 */

#include "nf/Executor.h"

#include <nf/Framework.h>
#include <nf/Logging.h>
#include <nf/RaiiToken.h>

#include <mutex>

using namespace nf;

namespace {

class ThisExecutorHolder
{
public:
    const std::shared_ptr<Executor> &lock(int sequenceNr) noexcept;

private:
    int m_sequenceNr{0};
    std::shared_ptr<Executor> m_executor;
};

const std::shared_ptr<Executor> &ThisExecutorHolder::lock(int sequenceNr) noexcept
{
    if (m_sequenceNr != sequenceNr) {
        m_executor = detail::g_framework->makeExecutor();
        m_sequenceNr = sequenceNr;
    }
    return m_executor;
}

class MainExecutorHolder
{
public:
    const std::shared_ptr<Executor> &lock(int sequenceNr) noexcept;

private:
    int m_sequenceNr{0};
    std::shared_ptr<Executor> m_executor;
    std::mutex m_mutex;
};

const std::shared_ptr<Executor> &MainExecutorHolder::lock(int sequenceNr) noexcept
{
    if (m_sequenceNr != sequenceNr) {
        std::unique_lock lock(m_mutex);
        if (m_sequenceNr != sequenceNr) {
            m_executor = Executor::thisThread();
            m_sequenceNr = sequenceNr;
        }
    }
    return m_executor;
}

} // anonymous namespace

const std::shared_ptr<Executor> &Executor::thisThread() noexcept
{
    thread_local ThisExecutorHolder s_executor;
    return s_executor.lock(Framework::sequenceNr());
}

const std::shared_ptr<Executor> &Executor::mainThread() noexcept
{
    static MainExecutorHolder s_executor;
    return s_executor.lock(Framework::sequenceNr());
}

Executor::Executor() noexcept
    : m_exceptionHandler([](const std::exception_ptr &eptr) { std::rethrow_exception(eptr); })
{
}

void Executor::setExceptionHandler(ExceptionHandler handler)
{
    m_exceptionHandler = std::move(handler);
}

int Executor::run()
try {
    m_threadId = std::this_thread::get_id();
    [[maybe_unused]] auto token =
        RaiiToken::nonDismissible([this] { m_threadId = std::thread::id(); });
    while (true) {
        try {
            return runImpl();
        } catch (const std::exception &ex) {
            nf::error("Executor {} encountered an exception ({})", fmt::ptr(this), ex.what());
            m_exceptionHandler(std::current_exception());
        } catch (...) {
            nf::error("Executor {} encountered an unknown exception", fmt::ptr(this));
            m_exceptionHandler(std::current_exception());
        }
    }
} catch (...) {
    stop(EXIT_FAILURE);
    discardAllPostedEvents();
    throw;
}

void Executor::post(Delay delay, Task &&task) noexcept
{
    if (constexpr auto zero = std::chrono::milliseconds(0); delay < zero) {
        delay = zero;
        nf::warn("Truncating negative post delay to 0");
    }
    postImpl(delay, std::move(task));
}

bool Executor::isThisThread() const noexcept
{
    return std::this_thread::get_id() == m_threadId;
}
