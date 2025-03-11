/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019-2023 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 *     Michał Ferenc <michal.ferenc@partner.bmw.de>
 *     Marcin Szczur <marcin.szczur@partner.bmw.de>
 *     Marko Popović <marko.popovic@bmw.de>
 */

#include "nf/BaseApplication.h"

#include <nf/ApplicationOptions.h>
#include <nf/Assert.h>
#include <nf/ChronoIoStream.h>
#include <nf/Executor.h>
#include <nf/Framework.h>
#include <nf/Logging.h>
#include <nf/NfVersion.h>
#include <nf/RaiiToken.h>
#include <nf/Singleton.h>
#include <nf/backend/StdoutLogger.h>

#ifdef NF_HAS_DLT
#include <nf/backend/DltLogger.h>
#endif

#include <boost/exception/diagnostic_information.hpp>
#include <boost/range/adaptor/reversed.hpp>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>

#include <unistd.h>
#include <systemd/sd-daemon.h>

#if __has_include(<cxxabi.h>)
#include <cxxabi.h>
#define NF_HAS_CXXABI
#endif

extern "C" {
// This variable contains the process name, defined by glibc, Linux only
extern char *__progname; // NOLINT(readability-identifier-naming, bugprone-reserved-identifier)
}

using namespace nf;

constexpr char HelpOptionKey[] = "help,h";
constexpr char HelpOptionCheckKey[] = "help";
constexpr char VersionOptionKey[] = "version,v";
constexpr char VersionOptionCheckKey[] = "version";
constexpr char StdOutOptionKey[] = "stdout";

namespace {

std::optional<std::chrono::milliseconds> getWatchdogTimeout() noexcept
{
    uint64_t wdgTimeout = 0;
    int ret = sd_watchdog_enabled(0, &wdgTimeout);
    if (ret < 0) {
        nf::error("sd_watchdog_enabled failed, err: {}", ret);
        return std::nullopt;
    }
    if (ret == 0) {
        /* If systemd watchdog is not configured then there is no timeout. */
        return std::nullopt;
    }
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::microseconds(wdgTimeout));
}

void ensureNonRootUser() noexcept
{
    if (getuid() == 0) {
        std::cerr << "This application cannot be run as root. Exiting." << std::endl;
        std::exit(EXIT_FAILURE); // NOLINT(concurrency-mt-unsafe)
    }
}

} // anonymous namespace

BaseApplication::BaseApplication(int argc, const char **argv) noexcept
    : m_options(std::make_unique<ApplicationOptions>(argc, argv))
{
}

BaseApplication::~BaseApplication() = default;

void BaseApplication::setLogId(std::string_view logId) noexcept
{
    m_logId = logId;
}

void BaseApplication::setDescription(std::string_view description) noexcept
{
    m_description = description;
}

void BaseApplication::setBuildInfo(std::string_view version, std::string_view variant,
                                   std::string_view environment) noexcept
{
    assertThat(!version.empty(), "Version must be specified!");
    m_version = version;
    m_version.append(" (");
    if (!variant.empty()) {
        m_version.append(variant).append(", ");
    }
    m_version.append("NF ").append(nf::NfVersion);
    if (!environment.empty()) {
        m_version.append(", ").append(environment);
    }
    m_version.append(")");
}

void BaseApplication::enablePeriodicBuildInfo(std::chrono::seconds interval)
{
    m_buildInfoInterval = interval;
}

void BaseApplication::ensureUniqueInstance() noexcept
{
    const std::string processName{__progname};
    if (processName.empty()) {
        std::cerr << "Failed to get application name. Exiting..." << std::endl;
        std::exit(EXIT_FAILURE); // NOLINT(concurrency-mt-unsafe)
    }

    auto lockFilePath{std::filesystem::temp_directory_path() / (processName + ".lock")};
    std::ofstream lockFileStream(lockFilePath);
    lockFileStream.close();

    try {
        m_uniqueInstanceLock =
            std::make_unique<boost::interprocess::file_lock>(lockFilePath.string().c_str());

        if (!m_uniqueInstanceLock->try_lock()) {
            std::cerr << "Another instance of this application already exists! Exiting..."
                      << std::endl;
            std::exit(EXIT_FAILURE); // NOLINT(concurrency-mt-unsafe)
        }
    } catch (const boost::interprocess::interprocess_exception &e) {
        std::cerr << "Exception: " << boost::diagnostic_information(e) << std::endl
                  << "Exiting..." << std::endl;
        std::exit(EXIT_FAILURE); // NOLINT(concurrency-mt-unsafe)
    } catch (const boost::exception &e) {
        std::cerr << "Exception: " << boost::diagnostic_information(e) << std::endl
                  << "Exiting..." << std::endl;
        std::exit(EXIT_FAILURE); // NOLINT(concurrency-mt-unsafe)
    } catch (...) {
        std::cerr << "Unknown exception. Exiting..." << std::endl;
        std::exit(EXIT_FAILURE); // NOLINT(concurrency-mt-unsafe)
    }
}

void BaseApplication::setSignalHandlers() noexcept
{
    const auto handleSignals{[this](const LinuxSignal &signal) {
        auto print = [](const std::string &s) {
            nf::info(s);
            std::cerr << s << std::endl;
        };

        switch (signal) {
        case LinuxSignal::SigInt:
        case LinuxSignal::SigTerm:
            print("Requesting application shutdown...");
            stop();
            break;
        default:
            nf::error("No POSIX signal handler available!");
            break;
        }
    }};

    Executor::mainThread()->setSignalHandlers(
        {{LinuxSignal::SigInt, handleSignals}, {LinuxSignal::SigTerm, handleSignals}});
}

void BaseApplication::resetSignalHandlers() const noexcept
{
    Executor::mainThread()->setSignalHandlers({});
}

int BaseApplication::run()
{
    setTerminateHandler();

    auto res = processOptions();
    if (res.hasValue() && !res.value()) {
        return EXIT_SUCCESS;
    }
    if (res.hasError()) {
        return res.errorValue();
    }

    if (m_isUniqueInstance) {
        ensureUniqueInstance();
    }

    if (m_isRootForbidden) {
        ensureNonRootUser();
    }

    initializeLogging();
    initializeFramework();
    startVersionLogging();

    if (!startApplicationHeartbeat()) {
        nf::warn("Application is started without watchdog heartbeat configured!");
    }

    setSignalHandlers();

    const auto &executor = Executor::mainThread();

    /* Initialize the context here, after the Framework has been initialized and the executors
     * have been created. If done before, it would fail with "cannot create with null executor". */
    m_context.emplace(executor);

    executor->post([this] { startUp().then(*m_context, [this] { notifyStarted(); }); });

    for (const auto &ctor : m_ctors) {
        ctor();
    }

    auto cleanupAndTerminate = RaiiToken::nonDismissible([this] {
        for (const auto &dtor : boost::adaptors::reverse(m_dtors)) {
            dtor();
        }

        resetSignalHandlers();

        SingletonRegistry::releaseAll();

        nf::info("Bye...");
        terminateFramework();
    });

    try {
        return executor->run();
    } catch (...) {
        nf::warn("caught unexpected exception, terminating the BaseApplication.");

        throw;
    }
}

void BaseApplication::stop(int exitCode) noexcept
{
    shutDown().then(*m_context, [exitCode] { Executor::mainThread()->stop(exitCode); });
}

void BaseApplication::allowMultipleInstances() noexcept
{
    m_isUniqueInstance = false;
}

void BaseApplication::allowRunAsRoot() noexcept
{
    m_isRootForbidden = false;
}

Result<bool, int>
BaseApplication::processCustomOptions(const ApplicationOptions & /*options*/) noexcept
{
    /* This can be overridden. Default behavior is to do nothing. */
    return true;
}

void BaseApplication::fillOptions(ApplicationOptions & /*options*/) noexcept
{
    /* This can be overridden. Default behavior is to do nothing. */
}

void BaseApplication::addInitializationStep(InitializationStep &&ctor,
                                            InitializationStep &&dtor) noexcept
{
    m_ctors.push_back(std::move(ctor));
    m_dtors.push_back(std::move(dtor));
}

void BaseApplication::initializeLogging() noexcept
{
    /* Set a corresponding logging backend. Handle the --stdout switch. */
    if (m_isLogStdout) {
        Logging::initialize(m_logId, {std::make_shared<backend::StdoutLogger>()});
    } else {
#ifdef NF_HAS_DLT
        Logging::initialize(m_logId,
                            {std::make_shared<backend::DltLogger>(m_logId.c_str(),
                                                                  m_description.c_str())});
#else
        Logging::initialize(m_logId, {});
#endif
    }
}

void BaseApplication::initializeFramework() noexcept
{
    Framework::initialize(std::make_unique<AsioFramework>());
}

void BaseApplication::terminateFramework() noexcept
{
    Framework::terminate();
}

void BaseApplication::setTerminateHandler() const noexcept
{
    /* It is static so it can be accessed from the handler below. */
    static std::terminate_handler s_originalHandler;

    s_originalHandler = std::set_terminate([]() noexcept {
        auto print = [](const auto &message) {
            std::cerr << "[NF] " << message << std::endl;
            nf::fatal(message);
        };

        /* Avoid endless recursion if this handler throws. */
        static bool s_isTerminating = false;
        if (s_isTerminating) {
            print("Terminate handler is called recursively");
            std::abort();
        }
        s_isTerminating = true;

        if (auto exception = std::current_exception()) {
            /* Print more details about the uncaught exception:
             *  - Exception type name.
             *  - Exception message for exceptions derived from std::exception. */

            std::string message = "Terminating because of uncaught exception";

#ifdef NF_HAS_CXXABI
            message.append(" of type ")
                .append(boost::core::demangle(abi::__cxa_current_exception_type()->name()));
#endif

            try {
                std::rethrow_exception(exception);
            } catch (const std::exception &ex) {
                message.append(": ").append(ex.what());
            } catch (...) {
                /* Do not add additional info for non-std exceptions. */
            }

            print(message);

        } else {
            /* std::terminate can be also explicitly called by a client code. In this case
             * there will be no active exception. */
            print("Terminating because std::terminate was explicitly called");
        }

        if (s_originalHandler) {
            s_originalHandler();
        } else {
            std::abort();
        }
    });
}

Result<bool, int> BaseApplication::processOptions() noexcept
{
    m_options->add(HelpOptionKey, "Show this help message");
    m_options->add(VersionOptionKey, "Print the version");
    m_options->add(StdOutOptionKey, "Print to stdout instead of DLT");

    fillOptions(*m_options);
    if (!m_options->parse()) {
        return result::Err{EXIT_FAILURE};
    }

    if (m_options->isSpecified(HelpOptionCheckKey)) {
        m_options->printHelp();
        return false;
    }

    if (m_options->isSpecified(VersionOptionCheckKey)) {
        if (m_version.empty()) {
            m_version = "(version not specified)";
        }
        std::cout << __progname << " " << m_version << std::endl;
        return false;
    }

    m_isLogStdout = m_options->isSpecified(StdOutOptionKey);

    auto res = processCustomOptions(*m_options);
    m_options.reset();

    return res;
}

bool BaseApplication::startApplicationHeartbeat() noexcept
{
    auto timeout = getWatchdogTimeout();
    if (!timeout) {
        nf::warn("Cannot read watchdog timeout");
        return false;
    }

    auto heartbeat = [] {
        if (int ret = sd_notify(0, "WATCHDOG=1"); ret < 0) {
            nf::error("Cannot reset watchdog service, err: {}", ret);
            /* Do not retrigger when heartbeat failed. */
            return false;
        }

        /* Timer task should be retriggered. */
        return true;
    };
    /* Immediately send an initial heartbeat. */
    heartbeat();

    /* Make timer for heartbeat with timeout / 2. */
    auto heartbeatInterval = timeout.value() / 2;
    m_watchdogTimer = Executor::mainThread()->makeTimer(heartbeatInterval, std::move(heartbeat));
    m_watchdogTimer->start();
    nf::info("Watchdog heartbeat is configured for {}", heartbeatInterval);

    return true;
}

void BaseApplication::notifyStarted() const noexcept
{
    if (int ret = sd_notify(0, "READY=1"); ret < 0) {
        nf::error("Cannot notify watchdog service, err: {}", ret);
    }
}

void BaseApplication::startVersionLogging() noexcept
{
    if (m_version.empty()) {
        nf::info("Hello, my name is {}!", __progname);
    } else {
        nf::info("Hello, my name is {} version {}!", __progname, m_version);

        /* Periodically print an application version containing git revision, SDK version used
         * to build it and other needed information required to find which application version
         * produced the traces. This helps analyzing bug reports, because a version information
         * provided by testers is not always accurate. */
        if (m_buildInfoInterval) {
            m_versionTimer = Executor::mainThread()->makeTimer(m_buildInfoInterval.value(), [this] {
                nf::info("{} version is {}!", __progname, m_version);
                return true;
            });
            m_versionTimer->start();
        }
    }
}
