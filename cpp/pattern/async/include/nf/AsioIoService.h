/*
 * BMW Neo Framework
 *
 * Copyright (C) 2020 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Alexey Larikov <alexey.larikov@bmw.de>
 */
#pragma once

#include <nf/Global.h>

#include <boost/asio/io_service.hpp>

NF_BEGIN_NAMESPACE

/**
 * @ingroup nf_Core
 * @brief Provides access to underlying ASIO io_service.
 *
 * Underlying io_service can only be accessed if ASIO is used as a backend. Attempt
 * to get io_service from any other backend will lead to std::terminate()
 */
class AsioIoService
{
public:
    /**
     * @brief Get an A executor for a caller's thread.
     */
    static boost::asio::io_service &thisThread() noexcept;

    /**
     * @brief Get an ASIO executor for the main thread.
     */
    static boost::asio::io_service &mainThread() noexcept;
};

NF_END_NAMESPACE
