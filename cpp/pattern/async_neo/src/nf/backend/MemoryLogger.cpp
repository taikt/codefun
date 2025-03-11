/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include "nf/backend/MemoryLogger.h"

#include <nf/ContainerAlgorithms.h>

#include <iomanip>

using namespace nf::backend;

bool MemoryLogger::Entry::operator==(const Entry &rhs) const noexcept
{
    return std::tie(id, level, message) == std::tie(rhs.id, rhs.level, rhs.message);
}

NF_BEGIN_NAMESPACE
namespace backend {

std::ostream &operator<<(std::ostream &os, const MemoryLogger::Entry &entry) noexcept
{
    return os << "{" << entry.id << ", " << LogBackend::toString(entry.level) << ", "
              << std::quoted(entry.message) << "}";
}

} // namespace backend
NF_END_NAMESPACE

void MemoryLogger::write(const LogContext &context, LogLevel level,
                         const std::string &message) noexcept
{
    write({context, level, message, {}});
}

void MemoryLogger::write(const LogEntry &logEntry) noexcept
{
    std::unique_lock lock(m_m);
    m_logEntries.push_back(logEntry);
}

std::vector<MemoryLogger::Entry> MemoryLogger::entries() const noexcept
{
    std::unique_lock lock(m_m);
    return transformToVector(m_logEntries, [](const auto &logEntry) {
        return MemoryLogger::Entry{logEntry.logContext.id, logEntry.logLevel, logEntry.message};
    });
}

std::vector<nf::LogEntry> MemoryLogger::logEntries() const noexcept
{
    std::unique_lock lock(m_m);
    return m_logEntries;
}

void MemoryLogger::clear() noexcept
{
    std::unique_lock lock(m_m);
    m_logEntries.clear();
}
