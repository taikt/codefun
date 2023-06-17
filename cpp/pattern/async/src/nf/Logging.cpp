/*
 * BMW Neo Framework
 *
 * Copyright (C) 2018-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 *     Szymon Wojtczak <szymon.wojtczak@globallogic.com>
 *     Nikolai Kutiavin <Nikolai.Kutiavin@bmw.de>
 */

#include "nf/Logging.h"

#include <nf/ContainerAlgorithms.h>
#include <nf/Enums.h>
#include <nf/backend/MemoryLogger.h>
#include <nf/backend/StdoutLogger.h>

#include <boost/config.hpp>

#include <algorithm>
#include <atomic>
#include <iostream>
#include <unordered_map>

#include <fmt/chrono.h>

using namespace nf;
using namespace detail;

NF_ENUM_DEFINE_UTILS(LogLevel, Fatal, Error, Warn, Info, Debug, Verbose)

NF_BEGIN_NAMESPACE

std::ostream &operator<<(std::ostream &os, const LogEntry &logEntry) noexcept
{
    return os << logEntry.formatLogMsg();
}

namespace detail {

BOOST_SYMBOL_VISIBLE int threadMnemo()
{
    static std::atomic_int s_nextMnemo{0};
    thread_local int s_currentMnemo = -1;

    if (s_currentMnemo < 0) {
        s_currentMnemo = s_nextMnemo++;
    }

    return s_currentMnemo;
}

} // namespace detail
NF_END_NAMESPACE

namespace {
/**
 * Represents a log backend used in case logs have to be saved temporarily when
 * the Logging is suspended or not initialized yet.
 */
class FallbackLogger : public backend::MemoryLogger
{
public:
    void write(const LogContext &context, LogLevel level,
               const std::string &message) noexcept override;
    void write(const LogEntry &logEntry) noexcept override;
    void logBufferedMessages() const noexcept;
};

// Storage for the backends, handles add, remove, get requests, also implement as `Composite`
// design pattern.
class LogBackendManager : public LogBackend
{
public:
    explicit LogBackendManager(std::vector<std::shared_ptr<LogBackend>> &&backends) noexcept;
    LogBackendManager() = default;

public:
    void add(std::shared_ptr<LogBackend> backend) noexcept;
    bool remove(const std::type_info &backendType, std::size_t index) noexcept;
    std::vector<std::shared_ptr<LogBackend>> list() const noexcept;

public:
    void suspendOutput() noexcept override;
    void resumeOutput() noexcept override;

public:
    std::optional<std::shared_ptr<LogBackend>> get(const std::type_info &backendType,
                                                   std::size_t index) const noexcept;
    bool empty() const noexcept;

    void write(const LogEntry &logEntry) noexcept override;
    void write(const LogContext &context, LogLevel level,
               const std::string &message) noexcept override;
    void registerContext(LogContext &context) noexcept override;

private:
    auto find(const std::type_info &backendType, std::size_t index) const noexcept
    {
        return std::find_if(std::cbegin(m_backends), std::cend(m_backends),
                            [&backendType,
                             index](const std::shared_ptr<LogBackend> &backend) mutable {
                                const auto &reference = *backend;
                                if (typeid(reference) == backendType) {
                                    if (index == 0) {
                                        return true;
                                    }
                                    --index;
                                }
                                return false;
                            });
    }

private:
    std::vector<std::shared_ptr<LogBackend>> m_backends;
};

// Hold the list of rules and provide `appliesTo`
class LogFilterRuleManager : boost::noncopyable
{
public:
    explicit LogFilterRuleManager(std::vector<LogFilterRule> &&filters) noexcept;
    LogFilterRuleManager() = default;

public:
    bool appliesTo(const char *msgContextId, LogLevel msgLevel,
                   const char *msgFileName) const noexcept;

private:
    std::vector<LogFilterRule> m_filters;
};

// Hold all other classes, control their lifetime
class LoggingSubsystem : boost::noncopyable
{
public:
    std::vector<LogContext *> contexts() const noexcept;
    // Not owning pointer, can be null
    LogBackendManager *backends() noexcept;
    LogFilterRuleManager &filters() noexcept;
    std::string appId() const noexcept;
    /*
     * Return backend that is available for the writing now.
     * In the case, when system hasn't been initialized yet or logging is suspended
     * this returns `FallbackLogger`, for other cases - BackendsStorage
     */
    LogBackend &sink() noexcept;
    bool isInitialized() const noexcept;

public:
    void initialize(std::unique_ptr<LogFilterRuleManager> &&filters,
                    std::unique_ptr<LogBackendManager> &&backends);
    bool addContext(LogContext *context);
    void setAppId(std::string_view id);
    void terminate() noexcept;

private:
    // Methods to maintain the invariant
    void setBackendManager(std::unique_ptr<LogBackendManager> &&backends);
    void resetBackendManager();

private:
    std::unique_ptr<LogFilterRuleManager> m_filters = std::make_unique<LogFilterRuleManager>();
    std::unique_ptr<LogBackendManager> m_backends;
    std::shared_ptr<FallbackLogger> m_fallbackLogger = std::make_shared<FallbackLogger>();
    std::unordered_map<std::string, LogContext *> m_contexts;
    std::string m_appId = "MYAP";
    std::optional<LogBackend *> m_currentSink;
};

LogBackendManager::LogBackendManager(std::vector<std::shared_ptr<LogBackend>> &&backends) noexcept
    : m_backends(std::move(backends))
{
}

void LogBackendManager::add(std::shared_ptr<LogBackend> backend) noexcept
{
    m_backends.push_back(std::move(backend));
}

bool LogBackendManager::remove(const std::type_info &backendType, std::size_t index) noexcept
{
    if (auto it = find(backendType, index); it != m_backends.cend()) {
        m_backends.erase(it);
        return true;
    }
    return false;
}

std::vector<std::shared_ptr<LogBackend>> LogBackendManager::list() const noexcept
{
    return m_backends;
}


void LogBackendManager::suspendOutput() noexcept
{
    for (auto &backend : m_backends) {
        backend->suspendOutput();
    }
}

void LogBackendManager::resumeOutput() noexcept
{
    for (auto &backend : m_backends) {
        backend->resumeOutput();
    }
}

std::optional<std::shared_ptr<LogBackend>> LogBackendManager::get(const std::type_info &backendType,
                                                                  std::size_t index) const noexcept
{
    if (auto it = find(backendType, index); it != m_backends.cend()) {
        return *it;
    }
    return {};
}

bool LogBackendManager::empty() const noexcept
{
    return m_backends.empty();
}

void LogBackendManager::write(const LogEntry &logEntry) noexcept
{
    for (auto &backend : m_backends) {
        backend->write(logEntry);
    }
}

void LogBackendManager::write(const LogContext &context, LogLevel level,
                              const std::string &message) noexcept
{
    write({context, level, message, {}});
}

void LogBackendManager::registerContext(LogContext &context) noexcept
{
    for (auto &backend : m_backends) {
        backend->registerContext(context);
    }
}

LogFilterRuleManager::LogFilterRuleManager(std::vector<LogFilterRule> &&filters) noexcept
    : m_filters(std::move(filters))
{
}

bool LogFilterRuleManager::appliesTo(const char *msgContextId, LogLevel msgLevel,
                                     const char *msgFileName) const noexcept
{
    return std::any_of(begin(m_filters), end(m_filters),
                       [msgContextId, msgLevel, msgFileName](const auto &filter) {
                           return filter.appliesTo(msgContextId, msgLevel, msgFileName);
                       });
}

std::vector<LogContext *> LoggingSubsystem::contexts() const noexcept
{
    std::vector<LogContext *> result;
    result.reserve(std::size(m_contexts));
    std::transform(std::begin(m_contexts), std::end(m_contexts), std::back_inserter(result),
                   [](const auto &pair) { return pair.second; });
    return result;
}

LogBackendManager *LoggingSubsystem::backends() noexcept
{
    return m_backends.get();
}

LogFilterRuleManager &LoggingSubsystem::filters() noexcept
{
    return *m_filters;
}

std::string LoggingSubsystem::appId() const noexcept
{
    return m_appId;
}

LogBackend &LoggingSubsystem::sink() noexcept
{
    return *(m_currentSink.value_or(m_fallbackLogger.get()));
}

bool LoggingSubsystem::isInitialized() const noexcept
{
    return static_cast<bool>(m_backends);
}

void LoggingSubsystem::initialize(std::unique_ptr<LogFilterRuleManager> &&filters,
                                  std::unique_ptr<LogBackendManager> &&backends)
{
    const bool initialized = isInitialized();
    m_filters = std::move(filters);
    setBackendManager(std::move(backends));

    /* The next code serves two purposes. First, it checks if the current thread is the
     * main thread, i.e. there were no logs from other threads. Second, it forces this
     * thread to be the main thread. This is useful if the initialization in one thread
     * is followed by a log from another thread. */
    if (threadMnemo() != 0) {
        nf::fatal("Initializing logging in a wrong thread...");
    }

    /* In case something was logged before the logging was initialized, we print it with
     * a warning to get some attention. */
    m_fallbackLogger->logBufferedMessages();
    m_fallbackLogger->clear();

    if (initialized) {
        nf::fatal("Logging must be terminated before it can be initialized again...");
    }
}


bool LoggingSubsystem::addContext(LogContext *context)
{
    auto [it, isInserted] = m_contexts.emplace(std::string(context->id), context);
    if (!isInserted) {
        return false;
    }

    if (m_backends) {
        m_backends->registerContext(*context);
    }
    return true;
}

void LoggingSubsystem::setAppId(std::string_view id)
{
    m_appId = id;
}

void LoggingSubsystem::terminate() noexcept
{
    resetBackendManager();
    m_filters = std::make_unique<LogFilterRuleManager>();
}

void LoggingSubsystem::setBackendManager(std::unique_ptr<LogBackendManager> &&backends)
{
    m_backends = std::move(backends);
    m_currentSink = m_backends.get();
}

void LoggingSubsystem::resetBackendManager()
{
    m_currentSink.reset();
    m_backends.reset();
}

LoggingSubsystem &instance()
{
    static LoggingSubsystem s_subsystem;
    return s_subsystem;
}

void FallbackLogger::write(const LogContext &context, LogLevel level,
                           const std::string &message) noexcept
{
    write({context, level, message, {}});
}

void FallbackLogger::write(const LogEntry &logEntry) noexcept
{
    backend::StdoutLogger{}.write(logEntry);
    MemoryLogger::write(logEntry);
}

void FallbackLogger::logBufferedMessages() const noexcept
{
    if (m_logEntries.empty()) {
        return;
    }

    nf::warn("These messages were logged before logging was initialized:");
    for (const auto &logEntry : m_logEntries) {
        nf::log(ActiveLogContext, logEntry.logLevel, " ==> [{}] {}", logEntry.logContext.id,
                logEntry.message);
    }
}
} // anonymous namespace



LogEntry::LogEntry(const LogContext &logContext, LogLevel logLevel, std::string_view message,
                   const source_location &location) noexcept
    : timePoint(std::chrono::system_clock::now())
    , appId(Logging::appId())
    , logContext(logContext)
    , logLevel(logLevel)
    , message(message)
    , location(location)
    , threadId(std::this_thread::get_id())
    , processId(getpid())
{
}

std::string LogEntry::formatLogMsg() const noexcept
{
    return fmt::format("{} | {:>4} | {:>4} | {:>7} | {} # {}:{} # {} # {}", formatTimeStamp(),
                       appId, logContext.id, LogBackend::toString(logLevel), message,
                       location.file_name(), location.line(), processId, threadId);
}

std::string LogEntry::formatTimeStamp() const noexcept
{
    auto ms =
        std::chrono::duration_cast<std::chrono::milliseconds>(timePoint.time_since_epoch()) % 1000;
    auto t = std::chrono::system_clock::to_time_t(timePoint);
    auto *lt = std::localtime(&t); // NOLINT(concurrency-mt-unsafe)

    return fmt::format("{:%d/%m/%Y %H:%M:%S}.{:03d}", *lt, ms.count());
}

bool LogEntry::operator==(const LogEntry &rhs) const noexcept
{
    return std::tie(appId, logContext.id, logLevel, message)
        == std::tie(rhs.appId, rhs.logContext.id, rhs.logLevel, rhs.message);
}

LogContext::LogContext(const char *id, const char *desc) noexcept
    : id(id)
    , desc(desc)
    , data(nullptr)
{

    const bool isInserted = instance().addContext(this);

    if (!isInserted) {
        std::cerr << "Logging context with id " << id << " already exists (" << desc
                  << "). Aborting..." << std::endl;
        std::abort();
    }
}

void LogBackend::registerContext(LogContext & /*context*/) noexcept
{
}

void LogBackend::suspendOutput() noexcept
{
}

void LogBackend::resumeOutput() noexcept
{
}

const char *LogBackend::toString(LogLevel level) noexcept
{
    return ::toString(level);
}

bool LogBackend::isSuspended() const noexcept
{
    return false;
}

LogFilterRule LogFilterRule::suppressContext(std::string contextId, LogLevel maxLevel,
                                             std::string fileName) noexcept
{
    return {std::move(contextId), maxLevel, std::move(fileName)};
}

LogFilterRule LogFilterRule::suppressLevelsUpTo(LogLevel maxLevel) noexcept
{
    return {{}, maxLevel, {}};
}

LogFilterRule LogFilterRule::suppressFile(std::string fileName, LogLevel maxLevel) noexcept
{
    return {{}, maxLevel, std::move(fileName)};
}

bool LogFilterRule::appliesTo(const char *msgContextId, LogLevel msgLevel,
                              const char *msgFileName) const noexcept
{
    if (!contextId.empty() && msgContextId && contextId != msgContextId) {
        return false;
    }

    const auto minMatchingLogIdx = _nf_enum_LogLevel::idx(maxLevel);
    if (minMatchingLogIdx > _nf_enum_LogLevel::idx(msgLevel)) {
        return false;
    }

    return fileName.empty() || (msgFileName && fileName == msgFileName);
}

bool Logging::isInitialized() noexcept
{
    return instance().isInitialized();
}

void Logging::initialize(std::vector<std::shared_ptr<LogBackend>> &&backends) noexcept
{
    initialize(std::move(backends), {});
}

void Logging::initialize(std::vector<std::shared_ptr<LogBackend>> &&backends,
                         std::vector<LogFilterRule> &&filters) noexcept
{
    instance().initialize(std::make_unique<LogFilterRuleManager>(std::move(filters)),
                          std::make_unique<LogBackendManager>(std::move(backends)));
}

void Logging::initialize(std::string_view appId,
                         std::vector<std::shared_ptr<LogBackend>> &&backends) noexcept
{
    instance().setAppId(appId);
    initialize(std::move(backends));
}

void Logging::initialize(std::string_view appId,
                         std::vector<std::shared_ptr<LogBackend>> &&backends,
                         std::vector<LogFilterRule> &&filters) noexcept
{
    instance().setAppId(appId);
    initialize(std::move(backends), std::move(filters));
}

void Logging::terminate() noexcept
{
    instance().terminate();
}

void Logging::add(std::shared_ptr<LogBackend> backend) noexcept
{
    if (!isInitialized()) {
        initialize({std::move(backend)});
    } else {
        instance().backends()->add(std::move(backend));
    }
}

std::vector<LogContext *> Logging::contexts() noexcept
{
    return instance().contexts();
}

std::vector<std::shared_ptr<LogBackend>> Logging::logBackends() noexcept
{
    if (isInitialized()) {
        return instance().backends()->list();
    }

    return {};
}

bool Logging::isEnabled(const LogContext & /*context*/, LogLevel /*level*/) noexcept
{
    return true;
}

void Logging::write(const LogContext &context, LogLevel level, const std::ostringstream &message,
                    const char *fileName, const char *funcName, int line) noexcept
{
    detail::log(context, level, "{}", fmt::make_format_args(message.str()),
                source_location::current(fileName, funcName, line));
}

std::string Logging::appId() noexcept
{
    return instance().appId();
}

void Logging::suspendOutput() noexcept
{
    if (!instance().isInitialized()) {
        nf::warn("Unable to suspend output of uninitialized logging");
        return;
    }
    instance().backends()->suspendOutput();
}

void Logging::resumeOutput() noexcept
{
    if (!instance().isInitialized()) {
        nf::warn("Unable to resume output of uninitialized logging");
        return;
    }


    instance().backends()->resumeOutput();
}

void detail::log(const LogContext &context, LogLevel level, fmt::string_view format,
                 fmt::format_args args, const source_location &location) noexcept
{
    // TODO(NODE0DEV-539): Revisit the filtering approach.
    if (instance().filters().appliesTo(context.id, level, location.file_name())) {
        return;
    }

    auto message = [&level, &format, &args] {
        try {
            return fmt::vformat(format, args);
        } catch (fmt::v8::format_error &ex) {
            level = LogLevel::Fatal;
            return fmt::format("Cannot log \"{}\": {}", format, ex.what());
        }
    };

    auto annotate = [mnemo = threadMnemo()](std::string &&message) {
        // TODO(NODE0DEV-155): Support custom message patterns.
        if (mnemo == 0) {
            return std::move(message);
        }
        return fmt::format("[{}] {}", mnemo, message);
    };

    auto write = [&context, &level, &location](const std::string &message) {
        instance().sink().write({context, level, message, location});
    };

    write(annotate(message()));
}

void Logging::remove(const std::type_info &backendType, std::size_t index) noexcept
{
    if (instance().isInitialized()) {
        if (instance().backends()->remove(backendType, index)) {
            if (instance().backends()->empty()) {
                instance().terminate();
            }
        }
    }
}

std::optional<std::shared_ptr<LogBackend>> Logging::backend(const std::type_info &backendType,
                                                            std::size_t index)
{
    if (instance().isInitialized()) {
        return instance().backends()->get(backendType, index);
    }
    return {};
}
