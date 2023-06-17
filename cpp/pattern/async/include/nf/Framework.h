/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019-2021 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 *     Thomas Leichtle <thomas.leichtle@bmw.de>
 */

#pragma once

#include <nf/Global.h>

#include <boost/core/noncopyable.hpp>

#include <memory>

NF_BEGIN_NAMESPACE

class Executor;

namespace commonapi {
class ContextHolder;
} // namespace commonapi

/**
 * @ingroup nf_Core
 * @brief The framework abstraction.
 *
 * This class provides an abstraction for the framework's core functionality which can be
 * implemented using different technologies such as boost::asio or Qt.
 *
 * This is a base class for such implementations (backends). This class is intended to be
 * used @b only by other NF classes, e.g. @ref Executor or @ref Timer. Framework users shall
 * not access this class directly except for @ref initialize() and @ref terminate().
 */
class Framework : boost::noncopyable
{
public:
    /**
     * @brief Initialize the framework with the provided backend.
     *
     * This function initializes the framework with the provided backend. The framework must
     * be initialized before @ref nf_core_Executors and @ref nf_core_Logging can be used.
     */
    static void initialize(std::unique_ptr<Framework> &&framework) noexcept;

    /**
     * @brief Terminate the framework.
     *
     * This function terminates (de-initializes) the framework. This includes stopping the
     * logging subsystem.
     */
    static void terminate() noexcept;

public:
    virtual ~Framework() = default;

    /**
     * @internal
     * @brief Construct an executor.
     */
    virtual std::shared_ptr<Executor> makeExecutor() noexcept = 0;

protected:
    Framework() noexcept = default;

private:
    friend class Executor;
    friend class commonapi::ContextHolder;
    static int sequenceNr() noexcept;
};

/**
 * @ingroup nf_Core
 * @brief The NF boost::asio backend.
 *
 * This backend is based on @c boost::asio::io_context object. See boost::asio <a href=
 * "https://www.boost.org/doc/libs/release/doc/html/boost_asio.html"> documentation</a> for
 * more details.
 */
class AsioFramework : public Framework
{
public:
    std::shared_ptr<Executor> makeExecutor() noexcept override;
};

namespace detail {
// NOLINTNEXTLINE(readability-identifier-naming)
extern std::unique_ptr<Framework> g_framework;
} // namespace detail

NF_END_NAMESPACE
