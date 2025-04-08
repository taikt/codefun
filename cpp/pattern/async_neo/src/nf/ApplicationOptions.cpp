/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 */

#include "nf/ApplicationOptions.h"

#include <nf/Logging.h>

#include <iostream>

using namespace nf;

ApplicationOptions::ApplicationOptions(int argc, const char **argv) noexcept
    : m_argc(argc)
    , m_argv(argv)
{
}

void ApplicationOptions::add(const char *optionName, const char *helpText) noexcept
{
    m_options.add_options()(optionName, helpText);
}

bool ApplicationOptions::isSpecified(const char *optionName) const noexcept
{
    return m_vm.count(optionName);
}

bool ApplicationOptions::parse() noexcept
{
    try {
        po::store(po::parse_command_line(m_argc, m_argv, m_options), m_vm);
        po::notify(m_vm);
        return true;
    } catch (const po::error &ex) {
        std::cerr << "Failed to parse command line options: " << ex.what() << std::endl;
        return false;
    }
}

void ApplicationOptions::printHelp() const noexcept
{
    std::cout << m_options;
}
