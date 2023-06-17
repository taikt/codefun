/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019-2020 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include "nf/backend/DltLogger.h"

#include <unordered_map>

using namespace nf;
using namespace backend;

namespace {

auto &dltContexts() noexcept
{
    static std::unordered_map<LogContext *, DltContext> s_contexts;
    return s_contexts;
}

void registerDltContext(LogContext *context) noexcept
{
    auto &contexts = dltContexts();
    auto [it, isNew] = contexts.emplace(std::make_pair(context, DltContext()));
    if (isNew) {
        /* Do not register the DLT context if a context with the same id was already
         * registered! */
        DLT_REGISTER_CONTEXT(it->second, context->id, context->desc);
        context->data = &it->second;
    }
}

void registerExistingContexts() noexcept
{
    for (auto *context : Logging::contexts()) {
        registerDltContext(context);
    }
}

} // anonymous namespace

DltLogger::DltLogger() noexcept
{
    registerExistingContexts();
}

DltLogger::DltLogger(const char *id, const char *desc) noexcept
    : m_isDltAppRegistered(true)
{
    DLT_REGISTER_APP(id, desc);
    registerExistingContexts();
}

DltLogger::~DltLogger()
{
    if (m_isDltAppRegistered) {
        DLT_UNREGISTER_APP();
    }
}

DltContext &DltLogger::toDlt(const LogContext &context) noexcept
{
    return *static_cast<DltContext *>(context.data);
}

DltLogLevelType DltLogger::toDlt(LogLevel level) noexcept
{
    switch (level) {
    case LogLevel::Fatal:
        return DLT_LOG_FATAL;
    case LogLevel::Error:
        return DLT_LOG_ERROR;
    case LogLevel::Warn:
        return DLT_LOG_WARN;
    case LogLevel::Info:
        return DLT_LOG_INFO;
    case LogLevel::Debug:
        return DLT_LOG_DEBUG;
    case LogLevel::Verbose:
        return DLT_LOG_VERBOSE;
    }
    return DLT_LOG_DEFAULT;
}

void DltLogger::registerContext(LogContext &context) noexcept
{
    registerDltContext(&context);
}

void DltLogger::write(const LogContext &context, LogLevel level,
                      const std::string &message) noexcept
{
    write({context, level, message, {}});
}

void DltLogger::write(const LogEntry &logEntry) noexcept
{
    auto &dltContext = toDlt(logEntry.logContext);
    auto dltLogLevel = toDlt(logEntry.logLevel);

    const std::size_t dltMessageSize = logEntry.message.size();
    const std::size_t maxLength = DLT_USER_BUF_MAX_SIZE
        - sizeof(std::uint16_t) /* space for message length */
        - sizeof(std::uint32_t) /* space for message id */
        - sizeof(std::uint32_t) /* space for message type info */
        - 1;                    /* space for terminating zero */

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"

    if (dltMessageSize <= maxLength) {
        DLT_LOG(dltContext, dltLogLevel, DLT_STRING(logEntry.message.c_str()));
    } else {
        /* If a message is longer than the DLT buffer size, then split it. Using
         * std::string::substr() is not super optimal, but this code will not be
         * executed very often, so let's keep it. */
        DLT_LOG(dltContext, dltLogLevel, DLT_STRING(logEntry.message.substr(0, maxLength).c_str()));

        /* All messages except the first one are prepended with "->" */
        const std::size_t chunkLength = maxLength - 3; /* length of "-> " */
        for (std::size_t start = maxLength; start < dltMessageSize; start += chunkLength) {
            DLT_LOG(dltContext, dltLogLevel,
                    DLT_STRING(std::string("-> ")
                                   .append(logEntry.message.substr(start, chunkLength))
                                   .c_str()));
        }
    }

#pragma GCC diagnostic pop
}
