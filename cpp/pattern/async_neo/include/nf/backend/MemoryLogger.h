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

#include <nf/Logging.h>

#include <mutex>

NF_BEGIN_NAMESPACE

namespace backend {

class MemoryLogger : public LogBackend
{
public:
    struct [[deprecated(
        "Please use Logging::LogEntry which is its more detailed counterpart.")]] Entry
    {
        std::string id;
        LogLevel level;
        std::string message;

        bool operator==(const Entry &rhs) const noexcept;
        friend std::ostream &operator<<(std::ostream &os, const Entry &entry) noexcept;
    };

public:
    void write(const LogContext &context, LogLevel level,
               const std::string &message) noexcept override;
    void write(const LogEntry &logEntry) noexcept override;
    [[deprecated("Please use logEntries() instead.")]] std::vector<Entry> entries() const noexcept;
    std::vector<LogEntry> logEntries() const noexcept;
    void clear() noexcept;

protected:
    std::vector<LogEntry> m_logEntries;
    mutable std::mutex m_m;
};

} // namespace backend

NF_END_NAMESPACE
