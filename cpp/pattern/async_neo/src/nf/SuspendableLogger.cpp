/*
 * BMW Neo Framework
 *
 * Copyright (C) 2023 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Nikolai Kutiavin <Nikolai.Kutiavin@bmw.de>
 */

#include "nf/SuspendableLogger.h"

using namespace nf;

void SuspendableLogger::write(const LogContext &context, LogLevel level,
                              const std::string &message) noexcept
{
    SuspendableLogger::write({context, level, message, {}});
}

void SuspendableLogger::write(const LogEntry &logEntry) noexcept
{
    if (isSuspended()) {
        m_memoryLogger.write(logEntry);
    } else {
        output(logEntry);
    }
}

void SuspendableLogger::suspendOutput() noexcept
{
    m_suspended = true;
    aboutToSuspend();
}

void SuspendableLogger::resumeOutput() noexcept
{
    aboutToResume();
    m_suspended = false;
    auto entries = m_memoryLogger.logEntries();
    for (const auto &entry : entries) {
        output(entry);
    }
    m_memoryLogger.clear();
}

bool SuspendableLogger::isSuspended() const noexcept
{
    return m_suspended;
}