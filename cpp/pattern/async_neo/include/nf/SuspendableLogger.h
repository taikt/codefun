/*
 * BMW Neo Framework
 *
 * Copyright (C) 2023 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Nikolai Kutiavin <Nikolai.Kutiavin@bmw.de>
 */

#pragma once

#include <nf/backend/MemoryLogger.h>

#include <memory.h>

NF_BEGIN_NAMESPACE

/**
 * @brief A wrapper for backends that captures messages in suspended mode.
 *
 * This backend is used as a base-class to implement those backends, that can be suspended.
 * Once suspended, it captures log entities to the memory, on resumption sends
 * the messages to the child class via 'hook'.
 *
 * To hold the class invariant it provides a list of the "hooks" marked as private:
 *  1. do log entry store,
 *  2. make some activities prior to a suspension,
 *  3. resume the backend.
 *
 * As an example, for the @c FileLogger those hooks can have the following application:
 *  1. write log entry to the file,
 *  2. flush all buffers and close a log file,
 *  3. open the log file for the writing.
 */
class SuspendableLogger : public nf::LogBackend
{
public:
    using nf::LogBackend::LogBackend;

public:
    void write(const LogContext &context, LogLevel level,
               const std::string &message) noexcept final;
    void write(const LogEntry &logEntry) noexcept final;
    void suspendOutput() noexcept final;
    void resumeOutput() noexcept final;
    bool isSuspended() const noexcept final;

private:
    /**
     * @brief Output a @c logEntry.
     *
     * A child class has to output an entry in this method. It's called if a backend is
     * not in the suspended state and @nref{LogBackend::write} is called, or if backend leaves the
     * suspended state and has to @c output all the captured messages in the
     * memory.
     *
     * @param logEntry Entry to log.
     */
    virtual void output(const LogEntry &logEntry) noexcept = 0;

    /**
     * @brief Hook for a suspension preparation.
     *
     * Allow to a child class to perform some actions before the backend is suspended,
     * for the example, close a file, flush buffers and others.
     */
    virtual void aboutToSuspend() noexcept = 0;

    /**
     * @brief Hook for a resumption.
     *
     * A callback for the child class to make some steps for the recovering from the suspended
     * state. It is called before the captured in memory messages are sent to a child class.
     * This hook is a good place to open the log file in the @c FileLogger.
     */
    virtual void aboutToResume() noexcept = 0;

private:
    bool m_suspended{false};
    nf::backend::MemoryLogger m_memoryLogger;
};

NF_END_NAMESPACE