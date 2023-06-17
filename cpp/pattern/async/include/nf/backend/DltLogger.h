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

#include <nf/Logging.h>

#include <dlt/dlt.h>

NF_BEGIN_NAMESPACE

namespace backend {

/**
 * @brief The DLT logger backend.
 */
class DltLogger final : public LogBackend
{
public:
    DltLogger() noexcept;
    DltLogger(const char *id, const char *desc) noexcept;
    ~DltLogger() override;

public:
    static DltContext &toDlt(const LogContext &context) noexcept;
    static DltLogLevelType toDlt(LogLevel level) noexcept;

public:
    void registerContext(LogContext &context) noexcept override;
    void write(const LogContext &context, LogLevel level,
               const std::string &message) noexcept override;
    void write(const LogEntry &logEntry) noexcept override;

private:
    bool m_isDltAppRegistered{false};
};

} // namespace backend

NF_END_NAMESPACE
