/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#pragma once

#include <nf/Logging.h>

NF_BEGIN_NAMESPACE

namespace backend {

class StdoutLogger final : public LogBackend
{
public:
    void write(const LogContext &context, LogLevel level,
               const std::string &message) noexcept override;
    void write(const LogEntry &logEntry) noexcept override;
};

} // namespace backend

NF_END_NAMESPACE
