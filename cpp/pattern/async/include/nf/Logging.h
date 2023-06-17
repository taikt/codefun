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

#pragma once

#include <nf/Global.h>

#include <boost/noncopyable.hpp>

#include <algorithm>
#include <chrono>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <thread>
#include <typeinfo>
#include <vector>

#include <experimental/source_location>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>

/**
 * @addtogroup nf_core_Logging
 * @{
 */

#define impl_NF_LOG_MKCTXNAME(ctxName) nf_logContext_##ctxName
#define impl_NF_LOG_ACTIVECTX []() -> const ::nf::LogContext & { return ActiveLogContext; }

/**
 * @name Log Context Management
 *
 * Define a new log context, import an existing one and set the current context to be
 * used by logging functions.
 *
 * @{
 */

/**
 * @hideinitializer
 * @brief Define a log context.
 * @param name A name of the log context.
 * @param id A log context id. Four symbols maximum.
 * @param desc A description for this log context.
 *
 * This macro defines the log context. The context with a given id can be defined only once.
 *
 * @p name is used to reference this log context. It must be unique within the application.
 * It must follow naming rules for variables, i.e. it must not contain quotation marks or
 * full stops.
 *
 * @p id is an identifier (@b not a string literal) which is at most four symbols long. It must
 * uniquely identify the log context and will be included into log messages. An attempt to define
 * two log contexts with the same id will either result in a link error (if this can be detected
 * at link time, e.g. if static libraries are used), or the application will be terminated at
 * runtime with @c std::abort().
 *
 * @see @ref nf_log_DefineContext
 *
 * @internal
 * @note The last line in this macro is needed to force a semicolon at the end of the macro
 *       when it is being used.
 * @note It is implemented as a function instead of a global variable because of linking
 *       issues.
 * @note LogContextIsDefined function is an additional check to verify that the same log context
 *       id is not defined twice.
 * @endinternal
 */
#define NF_LOG_DEFINE_CONTEXT(name, id, desc)                                                      \
    const ::nf::LogContext &impl_NF_LOG_MKCTXNAME(name)() noexcept                                 \
    {                                                                                              \
        static_assert(__builtin_strlen(#id) <= 4, "Context id must be at most four symbols long"); \
        static const ::nf::LogContext s_context(#id, desc);                                        \
        return s_context;                                                                          \
    }                                                                                              \
    void nf_LogContextIsDefined_##id()                                                             \
    {                                                                                              \
    }                                                                                              \
    static_assert(true, "Ensure a semicolon after this macro")

/**
 * @hideinitializer
 * @brief Import a log context.
 * @param name A name of the log context.
 *
 * This macro imports a log context which was defined in another translation unit.
 */
#define NF_LOG_IMPORT_CONTEXT(name) const ::nf::LogContext &impl_NF_LOG_MKCTXNAME(name)() noexcept

/**
 * @hideinitializer
 * @brief Get a log context object by its name.
 *
 * This macro converts a log context name to a @nfref{LogContext} object which represents
 * this log context. This macro must be used when one needs to provide a log context object
 * to some function.
 */
#define NF_LOG_GET_CONTEXT(name) impl_NF_LOG_MKCTXNAME(name)()

/**
 * @hideinitializer
 * @brief Use a log context.
 * @param name A name of the log context.
 *
 * This macro makes the given log context active within the current scope. It follows the
 * rules of the variable scope. This context will then be used for all subsequent log
 * messages in this scope.
 *
 * @see @ref NF_LOG_DEFINE_USE_CONTEXT()
 * @see @ref NF_LOG_IMPORT_USE_CONTEXT()
 */
#define NF_LOG_USE_CONTEXT(name)                                                                   \
    static const ::nf::LogContext &ActiveLogContext = NF_LOG_GET_CONTEXT(name);                    \
    [[maybe_unused]] constexpr auto fatal =                                                        \
        ::nf::detail::makeLogFunction(impl_NF_LOG_ACTIVECTX, ::nf::LogLevel::Fatal);               \
    [[maybe_unused]] constexpr auto error =                                                        \
        ::nf::detail::makeLogFunction(impl_NF_LOG_ACTIVECTX, ::nf::LogLevel::Error);               \
    [[maybe_unused]] constexpr auto warn =                                                         \
        ::nf::detail::makeLogFunction(impl_NF_LOG_ACTIVECTX, ::nf::LogLevel::Warn);                \
    [[maybe_unused]] constexpr auto info =                                                         \
        ::nf::detail::makeLogFunction(impl_NF_LOG_ACTIVECTX, ::nf::LogLevel::Info);                \
    [[maybe_unused]] constexpr auto debug =                                                        \
        ::nf::detail::makeLogFunction(impl_NF_LOG_ACTIVECTX, ::nf::LogLevel::Debug);               \
    [[maybe_unused]] constexpr auto verbose =                                                      \
        ::nf::detail::makeLogFunction(impl_NF_LOG_ACTIVECTX, ::nf::LogLevel::Verbose)

/**
 * @hideinitializer
 * @brief Define a log context and use it.
 *
 * This is a combination of @ref NF_LOG_DEFINE_CONTEXT() and @ref NF_LOG_USE_CONTEXT().
 */
#define NF_LOG_DEFINE_USE_CONTEXT(name, id, desc)                                                  \
    NF_LOG_DEFINE_CONTEXT(name, id, desc);                                                         \
    NF_LOG_USE_CONTEXT(name)

/**
 * @hideinitializer
 * @brief Import a log context and use it.
 *
 * This is a combination of @ref NF_LOG_IMPORT_CONTEXT() and @ref NF_LOG_USE_CONTEXT().
 */
#define NF_LOG_IMPORT_USE_CONTEXT(name)                                                            \
    NF_LOG_IMPORT_CONTEXT(name);                                                                   \
    NF_LOG_USE_CONTEXT(name)

/// @}

#define NF_LOG_FATAL(msg) NF_LOG_XLVL(::nf::LogLevel::Fatal, msg)
#define NF_LOG_ERROR(msg) NF_LOG_XLVL(::nf::LogLevel::Error, msg)
#define NF_LOG_WARN(msg) NF_LOG_XLVL(::nf::LogLevel::Warn, msg)
#define NF_LOG_INFO(msg) NF_LOG_XLVL(::nf::LogLevel::Info, msg)
#define NF_LOG_DEBUG(msg) NF_LOG_XLVL(::nf::LogLevel::Debug, msg)
#define NF_LOG_VERBOSE(msg) NF_LOG_XLVL(::nf::LogLevel::Verbose, msg)
#define NF_LOG_CUSTOM(lvl, msg, file, func, line)                                                  \
    NF_LOG_XCTX_DETAILED(ActiveLogContext, lvl, msg, file.c_str(), func.c_str(), line)

#define NF_LOG_FATAL_F(msg) NF_LOG_XLVL_F(::nf::LogLevel::Fatal, msg)
#define NF_LOG_ERROR_F(msg) NF_LOG_XLVL_F(::nf::LogLevel::Error, msg)
#define NF_LOG_WARN_F(msg) NF_LOG_XLVL_F(::nf::LogLevel::Warn, msg)
#define NF_LOG_INFO_F(msg) NF_LOG_XLVL_F(::nf::LogLevel::Info, msg)
#define NF_LOG_DEBUG_F(msg) NF_LOG_XLVL_F(::nf::LogLevel::Debug, msg)
#define NF_LOG_VERBOSE_F(msg) NF_LOG_XLVL_F(::nf::LogLevel::Verbose, msg)

#define NF_LOG_XLVL(level, msg) NF_LOG_XCTX(ActiveLogContext, level, msg)
#define NF_LOG_XLVL_F(level, msg) NF_LOG_XCTX_F(ActiveLogContext, level, msg)

#ifdef NF_DISABLE_LOG_MACRO_DEPRECATION
#define impl_NF_LOG_DEPRECATED
#else
#define impl_NF_LOG_DEPRECATED                                                                     \
    _Pragma(                                                                                       \
        "message (\"\
Logging macros are deprecated. Please use new functions like nf::info(). This message can be \
disabled by setting NF_DISABLE_LOG_MACRO_DEPRECATION CMake option to TRUE. This option is \
temporary to accommodate a migration from NF4 to NF5 and will be removed at some point.\")")
#endif

#define NF_LOG_XCTX(ctx, level, msg)                                                               \
    impl_NF_LOG_DEPRECATED;                                                                        \
    do {                                                                                           \
        const auto &_nf_ctx = ctx;                                                                 \
        const auto &_nf_level = level;                                                             \
        if (::nf::Logging::isEnabled(_nf_ctx, _nf_level)) {                                        \
            std::ostringstream _nf_os;                                                             \
            _nf_os << msg;                                                                         \
            ::nf::Logging::write(_nf_ctx, _nf_level, _nf_os, nullptr, nullptr, 0);                 \
        }                                                                                          \
    } while (!42)

#define NF_LOG_XCTX_F(ctx, level, msg)                                                             \
    NF_LOG_XCTX_DETAILED(ctx, level, msg, __FILE__, __FUNCTION__, __LINE__)
#define NF_LOG_XCTX_DETAILED(ctx, level, msg, file, function, line)                                \
    impl_NF_LOG_DEPRECATED;                                                                        \
    do {                                                                                           \
        const auto &_nf_ctx = ctx;                                                                 \
        const auto &_nf_level = level;                                                             \
        if (::nf::Logging::isEnabled(_nf_ctx, _nf_level)) {                                        \
            std::ostringstream _nf_os;                                                             \
            _nf_os << msg;                                                                         \
            const char *_nf_filename = ::nf::detail::extractFilename(file);                        \
            ::nf::Logging::write(_nf_ctx, _nf_level, _nf_os, _nf_filename, function, line);        \
        }                                                                                          \
    } while (!42)

/// @}

NF_BEGIN_NAMESPACE

using std::experimental::source_location;

enum class LogLevel
{
    Fatal,
    Error,
    Warn,
    Info,
    Debug,
    Verbose
};

//! Log context.
struct LogContext : boost::noncopyable
{
    LogContext(const char *id, const char *desc) noexcept;
    const char *id;
    const char *desc;
    void *data;
};

/**
 * @ingroup nf_core_Logging
 * @brief Detailed log information.
 *
 * This structure holds detailed log information as well as formatting and printing helpers.
 *
 * @since 5.4.0
 */
struct LogEntry
{
    std::chrono::time_point<std::chrono::system_clock>
        timePoint;                ///< A time point used for time stamp formatting.
    std::string appId;            ///< Application ID.
    const LogContext &logContext; ///< Log Context.
    LogLevel logLevel;            ///< Log severity level.
    std::string message;          ///< Log message.
    source_location location;     ///< Log location details.
    std::thread::id threadId;     ///< ID of thread which created the log.
    pid_t processId;              ///< ID of process which created the log.

    /**
     * @brief Construct a new @nfref{LogEntry} object.
     *
     * @param logContext Log Context.
     * @param logLevel Log severity level.
     * @param message Log message
     * @param location Log location details.
     */
    LogEntry(const LogContext &logContext, LogLevel logLevel, std::string_view message,
             const source_location &location) noexcept;

    /**
     * @brief Get a formatted string containing all log details.
     *
     * The format used is as follows:
     * @code
     * TimeStamp | AppID | ContextID | LogLevel | Message # File:Line # PID # ThreadID
     * @endcode
     *
     * @return Formatted string.
     */
    std::string formatLogMsg() const noexcept;

    /**
     * @brief Get a formatted time stamp.
     *
     * This functions converts stored @c time @c point into a human-readable time stamp.
     *
     * @return Formatted time stamp.
     */
    std::string formatTimeStamp() const noexcept;

    /**
     * @brief Compare two instances of @nfref{LogEntry}.
     *
     * @param rhs Instance of @nfref{LogEntry}
     * @return true if Application ID, Context ID, Log severity and message are the same.
     */
    bool operator==(const LogEntry &rhs) const noexcept;

    /**
     * @brief Streaming operator for @nfref{LogEntry}.
     */
    friend std::ostream &operator<<(std::ostream &os, const LogEntry &logEntry) noexcept;
};

/// Rule for filtering messages based on given criteria
struct LogFilterRule
{
    /**
     * @brief Create a rule which suppresses messages form a given context.
     * @param contextId Context identifier. Filter messages only from this context.
     * @param maxLevel Log level upper bound. Filter messages of this level or lower.
     * @param fileName Source file name. Filter messages only from this file.
     * @return Instance of a filter rule with given criteria for a by-context suppression.
     */
    static LogFilterRule suppressContext(std::string contextId,
                                         LogLevel maxLevel = nf::LogLevel::Fatal,
                                         std::string fileName = {}) noexcept;

    /**
     * @brief Create a rule which suppresses messages of certain log levels.
     * @param maxLevel Log level upper bound. Filter messages of this level of lower.
     * @return Instance of a filter rule with given log level criteria set accordingly.
     */
    static LogFilterRule suppressLevelsUpTo(LogLevel maxLevel) noexcept;

    /**
     * @brief Create a rule which suppresses messages from a given file.
     * @param fileName Source file name. Filter messages only from this file.
     * @param maxLevel Log level upper bound. Filter messages of this level or lower.
     * @return Instance of a filter rule with given criteria for a by-file suppression.
     */
    static LogFilterRule suppressFile(std::string fileName,
                                      LogLevel maxLevel = LogLevel::Fatal) noexcept;


    /**
     * @brief Check if rule applies to message of given attributes.
     * @param msgContextId Context identifier of evaluated message.
     * @param msgLevel Level of evaluated message.
     * @param msgFileName Optional name of a file where evaluate message is logged.
     * @return True when message meets all the criteria. False otherwise.
     */
    bool appliesTo(const char *msgContextId, LogLevel msgLevel,
                   const char *msgFileName) const noexcept;

    /// If not empty, limits the rule applicability to messages from given context only.
    std::string contextId;
    /// Limits the rule applicability only to messages not exceeding given log level.
    LogLevel maxLevel{LogLevel::Fatal};
    /// If not empty, limits the rule applicability to messages from given file only.
    std::string fileName;
};

class LogBackend
{
public:
    virtual ~LogBackend() = default;
    virtual void registerContext(LogContext &context) noexcept;
    [[deprecated("Please use write(LogEntry) overload instead.")]] virtual void
    write(const LogContext &context, LogLevel level, const std::string &message) noexcept = 0;
    virtual void write(const LogEntry &logEntry) noexcept = 0;

    /*
     * @brief Suspend logging to output.
     */
    virtual void suspendOutput() noexcept;
    /*
     * @brief Resume logging to output.
     */
    virtual void resumeOutput() noexcept;

    /*
     * @brief Is log backend suspended
     */
    virtual bool isSuspended() const noexcept;

    static const char *toString(LogLevel level) noexcept;
};

/**
 * @addtogroup nf_core_Logging
 * @{
 */

/**
 * @ingroup nf_Core
 * @brief Provides logging mechanisms.
 *
 * Class that enables initialization and termination of logging and managing log backends.
 */
class Logging
{
public:
    /**
     * @todo Document method
     */
    static bool isInitialized() noexcept;

    /**
     * @todo Document method
     */
    static void initialize(std::vector<std::shared_ptr<LogBackend>> &&backends) noexcept;

    /**
     * @todo Document method
     */
    static void initialize(std::vector<std::shared_ptr<LogBackend>> &&backends,
                           std::vector<LogFilterRule> &&filters) noexcept;
    static void initialize(std::string_view appId,
                           std::vector<std::shared_ptr<LogBackend>> &&backends) noexcept;
    static void initialize(std::string_view appId,
                           std::vector<std::shared_ptr<LogBackend>> &&backends,
                           std::vector<LogFilterRule> &&filters) noexcept;

    /**
     * @todo Document method
     */
    static void terminate() noexcept;

    /**
     * @todo Document method
     */
    static std::vector<LogContext *> contexts() noexcept;

    /**
     * @brief Return a list of backend registered in the logging subsystem.
     */
    static std::vector<std::shared_ptr<LogBackend>> logBackends() noexcept;

    /**
     * @brief Add a log backend.
     *
     * Adding first backend is an equivalent for @ref initialize().
     *
     * @warning This method is not thread-safe. Other threads may try to write the log to a logger
     * in the same time.
     *
     * @param backend Backend to add.
     */
    static void add(std::shared_ptr<LogBackend> backend) noexcept;

    /**
     * @brief Get the first available log backend of type T.
     *
     * If the backend is not found @e std::nullopt is returned.
     *
     * @return The log backend of type T
     */
    template <typename T>
    static auto backend() noexcept
    {
        return backend<T>(0);
    }

    /**
     * @brief Get the n-th backend of given type.
     *
     * If the backend is not found @e std::nullopt is returned.
     *
     * @param index An index of log backend of type @c T to be returned.
     * @return An optional with n-th log backend of type T or std::nullopt.
     */
    template <typename T>
    static auto backend(std::size_t index) noexcept
    {
        return backend(typeid(T), index);
    }

    /**
     * @brief Remove the first available log backend by type T.
     *
     * The method has the same effect as calling
     * @code
     * remove<T>(0);
     * @endcode
     *
     * @warning This method is not thread-safe. Other threads may try to write the log to a logger
     * in the same time.
     *
     */
    template <typename T>
    static void remove() noexcept
    {
        remove<T>(0);
    }

    /**
     * @brief Remove log backend by type and index.
     *
     * Removing last log backend is an equivalent for @ref terminate().
     *
     * @warning This method is not thread-safe. Other threads may try to write the log to a logger
     * in the same time.
     *
     * @param index An index of log backend of type @c T to remove.
     */
    template <typename T>
    static void remove(std::size_t index) noexcept
    {
        remove(typeid(T), index);
    }

    /**
     * @todo Document method
     */
    static bool isEnabled(const LogContext &context, LogLevel level) noexcept;

    /**
     * @todo Document method
     */
    static std::string appId() noexcept;

    /**
     * @brief Write a log message.
     *
     * Write a log message to available loggers. If the Logger was not initialized the messages
     * are written to a fallback logger.
     * @warning This method is not thread-safe.
     * Other threads may try to remove log backend that is written to.
     *
     * @todo Document parameters
     */
    static void write(const LogContext &context, LogLevel level, const std::ostringstream &message,
                      const char *fileName, const char *funcName, int line) noexcept;

    /**
     * @brief Suspends the output of log backends.
     *
     * Suspends output of all log backends that support it. If @c Logging is suspended the log
     * messages will be saved to a memory buffer. Calling this method when the @c Logging
     * is uninitialized or suspended will have no effect.
     *
     * @warning This method is not thread-safe.
     * Other threads may try to remove log backend that is being suspended.
     *
     * @see @ref nf_log_Suspend
     */
    static void suspendOutput() noexcept;

    /**
     * @brief Resumes the output of log backends.
     *
     * Resumes output of all log backends that support it. On resume all memory saved messages
     * will be written to suspended log backends. Calling this method when the @c Logging
     * is uninitialized or not suspended will have no effect.
     *
     * @warning This method is not thread-safe.
     * Other threads may try to remove log backend that is being output-resumed.
     *
     * @see @ref nf_log_Suspend
     */
    static void resumeOutput() noexcept;

private:
    static void remove(const std::type_info &backendType, std::size_t index) noexcept;
    static std::optional<std::shared_ptr<LogBackend>> backend(const std::type_info &backendType,
                                                              std::size_t index);
};

/// @}

namespace detail {

/**
 * @internal
 * @brief Extracts the filename component out of a path name.
 *
 * This method searches forwards in the specified @p path for UNIX path separators
 * and returns the substring of characters after the last path separator of the
 * input string. If the last character in @p path is a path separator, an empty
 * string is returned. If no path separator is found, @p path is returned. If
 * @p path contains an empty string, an empty string is returned.
 *
 * @p path must not be @c nullptr.
 */
constexpr const char *extractFilename(const char *path)
{
    const char *file = path;
    for (; *path != '\0'; path++) {
        if (*path == '/') {
            file = path + 1;
        }
    }
    return file;
}

/* This structure is needed to be able to get a default constructed source_location
 * at a place where logging functions are invoked. Such functions cannot have it
 * as a separate argument at the end with a default value because they are variadic
 * templates and you cannot have a defaulted argument after a variadic argument. */
struct FormatAndLocation
{
    fmt::string_view format;
    nf::source_location location;

    template <typename tFormat,
              std::enable_if_t<std::is_convertible_v<const tFormat &, fmt::string_view>, int> = 0>
    FormatAndLocation(tFormat &&format, // NOLINT
                      nf::source_location location = nf::source_location::current())
        : format(std::forward<tFormat>(format))
        , location(location)
    {
    }
};

void log(const LogContext &context, LogLevel level, fmt::string_view format, fmt::format_args args,
         const source_location &location) noexcept;

constexpr auto makeLogFunction(const LogContext &(*context)(), LogLevel level) noexcept
{
    return [context, level](FormatAndLocation fl, auto &&...args) noexcept {
        detail::log(context(), level, fl.format,
                    fmt::make_format_args(std::forward<decltype(args)>(args)...), fl.location);
    };
}

} // namespace detail

/* An alias for public interfaces. Clients do not need to know the "location" part
 * of this structure because if is an implementation detail. */
using FormatString = detail::FormatAndLocation;

/**
 * @ingroup nf_core_Logging
 * @brief Log a message using a custom log context and level.
 *
 * This functions logs a message with the given @p format and @p args using the
 * provided log @p context and @p level.
 *
 * @since 5.0
 */
template <typename... tArgs>
void log(const LogContext &context, LogLevel level, FormatString format, tArgs &&...args) noexcept
{
    detail::log(context, level, format.format, fmt::make_format_args(args...), format.location);
}

template <typename... tArgs>
void fatal(const LogContext &context, FormatString format, tArgs &&...args) noexcept
{
    log(context, LogLevel::Fatal, format, std::forward<tArgs>(args)...);
}

template <typename... tArgs>
void error(const LogContext &context, FormatString format, tArgs &&...args) noexcept
{
    log(context, LogLevel::Error, format, std::forward<tArgs>(args)...);
}

template <typename... tArgs>
void warn(const LogContext &context, FormatString format, tArgs &&...args) noexcept
{
    log(context, LogLevel::Warn, format, std::forward<tArgs>(args)...);
}

template <typename... tArgs>
void info(const LogContext &context, FormatString format, tArgs &&...args) noexcept
{
    log(context, LogLevel::Info, format, std::forward<tArgs>(args)...);
}

template <typename... tArgs>
void debug(const LogContext &context, FormatString format, tArgs &&...args) noexcept
{
    log(context, LogLevel::Debug, format, std::forward<tArgs>(args)...);
}

template <typename... tArgs>
void verbose(const LogContext &context, FormatString format, tArgs &&...args) noexcept
{
    log(context, LogLevel::Verbose, format, std::forward<tArgs>(args)...);
}

NF_END_NAMESPACE

#ifdef NF_LOG_CONTEXT

NF_LOG_IMPORT_CONTEXT(NF_LOG_CONTEXT);

NF_BEGIN_NAMESPACE

/* A reference to const does not count as constant, according to clang-tidy.
 * NOLINTNEXTLINE(readability-identifier-naming) */
static const LogContext &ActiveLogContext = NF_LOG_GET_CONTEXT(NF_LOG_CONTEXT);

template <typename... tArgs>
void fatal(FormatString format, tArgs &&...args) noexcept
{
    fatal(ActiveLogContext, format, std::forward<tArgs>(args)...);
}

template <typename... tArgs>
void error(FormatString format, tArgs &&...args) noexcept
{
    error(ActiveLogContext, format, std::forward<tArgs>(args)...);
}

template <typename... tArgs>
void warn(FormatString format, tArgs &&...args) noexcept
{
    warn(ActiveLogContext, format, std::forward<tArgs>(args)...);
}

template <typename... tArgs>
void info(FormatString format, tArgs &&...args) noexcept
{
    info(ActiveLogContext, format, std::forward<tArgs>(args)...);
}

template <typename... tArgs>
void debug(FormatString format, tArgs &&...args) noexcept
{
    debug(ActiveLogContext, format, std::forward<tArgs>(args)...);
}

template <typename... tArgs>
void verbose(FormatString format, tArgs &&...args) noexcept
{
    verbose(ActiveLogContext, format, std::forward<tArgs>(args)...);
}

NF_END_NAMESPACE

#endif
