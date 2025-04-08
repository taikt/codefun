/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include "nf/backend/StdoutLogger.h"

#include <iostream>

using namespace nf::backend;

void StdoutLogger::write(const LogContext &context, LogLevel level,
                         const std::string &message) noexcept
{
    write({context, level, message, {}});
}

void StdoutLogger::write(const LogEntry &logEntry) noexcept
{
    std::cout << logEntry.formatLogMsg() << std::endl;
}
