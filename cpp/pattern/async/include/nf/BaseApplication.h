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
 *     Manuel Nickschas <manuel.nickschas@bmw.de>
 */

#pragma once

#include <nf/Context.h>
#include <nf/Future.h>
#include <nf/Result.h>
#include <nf/Subscription.h>
#include <nf/Timer.h>

#include <boost/core/noncopyable.hpp>
#include <boost/interprocess/sync/file_lock.hpp>

#include <functional>
#include <memory>
#include <optional>

NF_BEGIN_NAMESPACE

class ApplicationOptions;

/**
 * @ingroup nf_core_BaseApp
 * @brief The base class for an NF-enabled application.
 *
 * @par General Usage
 *
 * A @nfref{BaseApplication} instance acts as the core part of an application. It manages
 * initialization, startup and shutdown of the application, and provides support for common
 * tasks such as parsing command-line arguments, setting POSIX signal handlers, and triggering
 * the systemd watchdog. It also sets up the main thread's executor, thus enabling the handling
 * of events.
 *
 * The recommended way to create an NF-enabled application is to inherit from
 * @nfref{BaseApplication} (or its lifecycle-enhanced variant, @nfref{lifecycle::NsmApplication}),
 * adding customizations in the derived class, instantiating it in @c main(), and returning the
 * result from @nfref{BaseApplication::run()} as the exit code.
 *
 * @code
 * // MyApplication.h
 *
 * #pragma once
 *
 * #include <nf/ApplicationOptions.h>
 * #include <nf/BaseApplication.h>
 * #include <nf/Result.h>
 *
 * class IBusinessLogic;
 *
 * class MyApplication : public nf::BaseApplication
 * {
 * public:
 *     BaseApplication(int argc, const char **argv) noexcept;
 *
 * private: // Handle command line arguments
 *     void fillOptions(nf::ApplicationOptions &options) noexcept override;
 *     nf::Result<bool, int>
 *     processCustomOptions(const nf::ApplicationOptions &options) noexcept override;
 *
 * private: // Startup and shutdown
 *     nf::Future<void> startUp() noexcept override;  // Should create business logic
 *     nf::Future<void> shutDown() noexcept override; // Should destroy business logic
 *
 * private:
 *     std::unique_ptr<IBusinessLogic> m_bl;
 * };
 * @endcode
 * @code
 * // main.cpp
 *
 * #include "MyApplication.h"
 *
 * #include<nf/BaseApplication.h>
 *
 * int main(int argc, const char *argv[])
 * {
 *     MyApplication app{argc, argv};
 *     return app.run();
 * }
 * @endcode
 * See the @ref nf_tut_DemoApp for a more detailed example.
 *
 * @anchor nf_core_BaseApp_InitStart
 * @par Initialization and Startup
 *
 * The initialization of a @ref BaseApplication happens in several stages:
 * - At construction time, static configuration is provided by calling methods such as
 *   @ref setLogId(), @ref setBuildInfo(), or @ref setDescription(). Such methods are
 *   typically invoked from the constructor of a custom class derived from @ref BaseApplication.
 * - Once @ref run() is invoked, the Framework is @ref nf::Framework::initialize() "initialized"
 *   and the main thread's @nfref{Executor} is created.
 * - Immediately before event processing starts, but already with a fully initialized Framework,
 *   a sequence of @ref addInitializationStep() "initialization steps" is invoked, if any are
 *   defined. For example, @nfref{lifecycle::NsmApplication} uses this to set up its communication
 *   to @c NodeStateManager.
 * - Finally, once the event loop is up and running, @ref startUp() is invoked through the same.
 *   This method should be overridden by the user in order to setup and start the application's
 *   business logic.
 *
 * @note @ref Context objects rely on the existence of an @nfref{Executor} in the main thread,
 *       and thus they cannot be created before initialization of the Framework is complete and
 *       event handling is up and running. Thus, it is unwise to create an application's business
 *       logic together with the @ref BaseApplication instance - doing so would cause an assertion.
 *       Instead, the business logic and any other object relying on an initialized Framework
 *       should be created in the overridden @ref startUp() method.
 *
 * @anchor nf_core_BaseApp_Shutdown
 * @par Shutdown
 *
 * Shutdown of an NF-enabled application can be triggered by a POSIX signal such as @c SIGTERM,
 * or programmatically by invoking @ref stop(). A clean shutdown goes through the following
 * sequence of steps:
 * - @ref shutDown() is invoked through the event loop. This method should be overridden by the
 *   user and take care of terminating the business logic and destroying any objects that rely
 *   on functioning event processing, such as CommonAPI proxies.
 * - Once the @ref shutDown() method has fulfilled its promise, event processing stops.
 * - Immediately afterwards, finalization actions registered via @ref addInitializationStep(),
 *   if any, are invoked in reverse order.
 * - Finally, the Framework is torn down, including the logging facilities. Singletons
 *   registered via @nfref{SingletonHolder} are destroyed.
 * - Afterwards, @ref run() returns and @c main() should be exited.
 *
 * @note Once @ref run() returns, the application object is no longer usable, and all objects
 *       relying on the Framework should have been destroyed at this point.
 *       Avoid storing things like CommonAPI proxies or @nfref{Context} instances in global
 *       variables (directly or indirectly), do not log during the destruction of global static
 *       objects, and be sure to destroy any relevant instances in @ref shutDown().
 *
 * @par Command Line Arguments
 *
 * This class defines these default command line arguments:
 * - @c \--help or @c -h -- print a help/usage message.
 * - @c \--version or @c -v -- print a @ref setBuildInfo "version string".
 * - @c \--stdout -- log to @e stdout instead of DLT.
 *
 * If the subclass needs to support additional arguments, it needs to override @ref fillOptions()
 * and @ref processCustomOptions() methods.
 *
 * @par POSIX Signal Handling
 *
 * This class handles the following POSIX system signals:
 * - @c SIGINT -- Call @ref shutDown() method and gracefully exit.
 * - @c SIGTERM -- Call @ref shutDown() method and gracefully exit.
 */
class BaseApplication : boost::noncopyable
{
public:
    using InitializationStep = std::function<void()>;

public:
    /**
     * @brief Constructor.
     * @param argc Count of arguments.
     * @param argv List of arguments.
     *
     * Constructs an NF application.
     *
     * The @p argc and @p argv arguments are processed by the application and they have the
     * same semantics as arguments of the @c main() function. The data referred to by these
     * arguments must stay valid for the entire lifetime of the object.
     */
    BaseApplication(int argc, const char **argv) noexcept;

    /// Destructor.
    virtual ~BaseApplication();

public:
    /**
     * @brief Set the application logging id.
     *
     * Set a logging id (up to four letters) to be used by the application. Logging will
     * be initialized with this id.
     */
    void setLogId(std::string_view logId) noexcept;

    /**
     * @brief Set the application description.
     *
     * Set a short string to describe the application. Logging will be initialized with
     * this description.
     */
    void setDescription(std::string_view description) noexcept;

    /**
     * @brief Set the build information.
     * @param version Build version.
     * @param variant Build variant (can be empty).
     * @param environment Build environment (can be empty).
     *
     * Set a build information to be periodically logged in the following format:
     * @code
     * process-name version is <version> ([<variant>, ]NF nf-version[, <environment>])!
     * @endcode
     *
     * It is recommended to use values provided by @ref nf_make_version_h(), e.g.
     * @code
     * setBuildInfo(my::PackageVersion,
     *              my::DeploymentVariant,
     *              my::BuildEnvironment);
     * @endcode
     *
     * @since 3.10
     */
    void setBuildInfo(std::string_view version, std::string_view variant = {},
                      std::string_view environment = {}) noexcept;

    /**
     * @brief Enable periodic logging of specified Build Info.
     *
     * Until this function is called, periodic logging of Build Info is disabled. Afterwards
     * the info is being logged every specified interval, or, if none given, every 30 seconds.
     *
     * @param interval Time in seconds between subsequent Build Info logs.
     */
    void enablePeriodicBuildInfo(std::chrono::seconds interval = std::chrono::seconds{30});

    /**
     * @brief Run the application.
     *
     * This method:
     * -# @ref nf::Framework::initialize() "Initializes" the Neo Framework.
     * -# Initializes the logging subsystem.
     * -# Handles command-line arguments.
     * -# Invokes @ref addInitializationStep() "initialization actions", if any.
     * -# Starts the main event loop.
     * -# Invokes @ref startUp().
     * -# Will throw if the @c Executor throws.
     *
     * This method blocks until the application is stopped. Once this happens, the following
     * steps are performed:
     * -# Invokes @ref shutDown(). This is skipped in case the application is stopped due to
     *    an exception or by direct invocation of @nfref{Executor::stop()} (as opposed to
     *    invoking @nfref{BaseApplication::stop()}).
     * -# Stops the main event loop.
     * -# Invokes @ref addInitializationStep() "finalization actions", if any.
     * -# Resets POSIX signal handlers.
     * -# Releases all singletons registered through @nfref{SingletonHolder}.
     * -# @ref nf::Framework::terminate() "Terminates" the Neo Framework.
     * -# Returns control to the caller (typically, the @c main() method).
     *
     * @note This method must be called before the creation of any business logic objects, in
     *       particular the premature creation of @nfref{Context} would cause an assertion.
     *
     * See @ref nf_core_BaseApp_InitStart "Initialization and Startup" for more detail on the
     * initialization sequence, and @ref nf_core_BaseApp_Shutdown "Shutdown" for the opposite.
     *
     * @return Exit code of the application.
     */
    int run();

    /**
     * @brief Stop the application.
     *
     * This first invokes @ref shutDown(), then stops event processing and tears down the
     * Framework, ultimately causing the return of @ref run() with the provided exit code.
     *
     * See @ref nf_core_BaseApp_Shutdown "Shutdown" for more details.
     *
     * @note Always use this method for stopping the application, and avoid invoking
     *       @nfref{Executor::stop()} directly. Doing so would skip several steps of
     *       the termination sequence, such as invoking @ref shutDown().
     *
     * @param exitCode Exit code of the application.
     */
    void stop(int exitCode = EXIT_SUCCESS) noexcept;

    /**
     * @brief Allow multiple instances of an application.
     *
     * Allows to run multiple instances of this application. By default only one
     * instance of the application can be run at the same time.
     *
     * @note To enable multiple application instances, it's necessary to call this function before
     * calling @e run.
     */
    void allowMultipleInstances() noexcept;

    /**
     * @brief Allow running the application as root.
     *
     * Allows running the application under the root user. By default applications are not
     * allowed to run as root.
     *
     * @note To allow running as root, it's necessary to call this function before calling @e run.
     *
     * @since 5.0
     */
    void allowRunAsRoot() noexcept;

protected:
    /**
     * @brief Initializes framework backend for the application.
     *
     * A subclass may override this method and implement here initialization of a framework
     * backend. If a subclass does not override this method, AsioFramework backend is initialized by
     * default. This method is executed early on during the run() phase. This method must not block.
     */
    virtual void initializeFramework() noexcept;

    /**
     * @brief Start up the application.
     *
     * A subclass must override this method and implement here its startup logic. This
     * method is invoked after all the initialization is done and the main loop is started.
     * The implementation must initiate a startup sequence and fulfill its @e promise when
     * it is completed. This method can be used for initializing
     * the business logic of the application. This method must not block.
     *
     * @see @ref nf_core_BaseApp_InitStart "Initialization and Startup".
     */
    virtual Future<void> startUp() noexcept = 0;

    /**
     * @brief Shut down the application.
     *
     * A subclass must override this method and implement here its shutdown logic. The
     * implementation must initiate a shutdown sequence and fulfill its @e promise when it
     * is completed. This method must not block.
     *
     * @note Usually, the platforms supported by Neo Framework require applications to terminate
     *       within a few seconds after being requested to do so. Thus, it should be ensured that
     *       invocation of this method does not take longer than that to fulfill its promise.
     *
     * @see @ref nf_core_BaseApp_Shutdown "Shutdown".
     */
    virtual Future<void> shutDown() noexcept = 0;

    /**
     * @brief Process custom options.
     *
     * The subclasses may override this method, and retrieve their own command option values by
     * calling the functions @ref ApplicationOptions::isSpecified() and @ref
     * ApplicationOptions::value() on options argument. Based on the read values, the subclass can
     * decide whether to continue execution by returning true or false from this function.
     *
     * @param options The ApplicationOptions object from which child classes can retrieve the
     * option values.
     * @return @ok true if the application can continue with execution.
     * @return @err Exit code of the application.
     */
    virtual Result<bool, int> processCustomOptions(const ApplicationOptions &options) noexcept;

    /**
     * @brief Specify the command line options of the subclass.
     *
     * The subclasses should override this method. From here, they should use
     * @ref ApplicationOptions::add() overloads on options argument to specify their own
     * custom command options.
     */
    virtual void fillOptions(ApplicationOptions &options) noexcept;

protected:
    /**
     * @brief Add initialization and finalization steps.
     *
     * This method adds an initialization action (@p ctor) to be executed before right before
     * the main event loop is started (but after @ref startUp() has been scheduled), and a
     * finalization action (@p dtor) to be executed right after the main event loop exists.
     *
     * @note While initialization actions will be invoked before the event loop is started,
     *       anything posted to the event loop will be scheduled after the invocation of the
     *       @ref startUp() method, which is the very first thing to be queued.
     *
     * @see @ref nf_core_BaseApp_InitStart "Initialization and Startup" and
     *      @ref nf_core_BaseApp_Shutdown "Shutdown".
     */
    void addInitializationStep(InitializationStep &&ctor, InitializationStep &&dtor) noexcept;

protected:
    std::optional<Context> m_context;

private: /* for unit testing */
    virtual void initializeLogging() noexcept;
    virtual void terminateFramework() noexcept;

private:
    void setTerminateHandler() const noexcept;

    /**
     * @brief Process the command line options.
     *
     * This function sets the command line options and parse the command options
     * passed into the application.
     *
     * @return @ok true if the application can continue with execution.
     * @return @err Exit code of the application.
     */
    Result<bool, int> processOptions() noexcept;

    /**
     * @brief Starts watchdog related activity for application.
     *
     * This function registers to the systemd watchdog and starts timer for refreshing the watchdog
     * service with watchdog timeout/2 interval.
     *
     * @return true, if watchdog functions has been successfully started.
     */
    bool startApplicationHeartbeat() noexcept;

    /* Notify systemd that this application has been fully started. */
    void notifyStarted() const noexcept;

    /**
     * @brief Start periodically logging an application version.
     */
    void startVersionLogging() noexcept;

    /**
     * @brief Prevents multiple application instances from running
     *
     * This function ensures only one instance of a process is running. In case multiple instances
     * exist, function exits immediately by means @e std::exit(EXIT_FAILURE).
     */
    void ensureUniqueInstance() noexcept;

    /**
     * @brief Sets signal handlers to be handled by the application.
     */
    virtual void setSignalHandlers() noexcept;

    /**
     * @brief Clears all already set signal handlers.
     */
    void resetSignalHandlers() const noexcept;

private:
    bool m_isLogStdout{false};
    bool m_isUniqueInstance{true};
    bool m_isRootForbidden{true};
    std::string m_logId;
    std::string m_description;
    std::string m_version;
    std::optional<std::chrono::seconds> m_buildInfoInterval;

    std::unique_ptr<ApplicationOptions> m_options;
    std::vector<InitializationStep> m_ctors;
    std::vector<InitializationStep> m_dtors;
    std::unique_ptr<Timer> m_watchdogTimer;
    std::unique_ptr<Timer> m_versionTimer;
    std::unique_ptr<boost::interprocess::file_lock> m_uniqueInstanceLock;
};

NF_END_NAMESPACE
