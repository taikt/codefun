/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019-2020 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Safia Salim <safia.salim@partner.bmw.de>
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 *     Thomas Leichtle <thomas.leichtle@bmw.de>
 */

#pragma once

#include <nf/Global.h>

#include <boost/core/noncopyable.hpp>
#include <boost/program_options.hpp>

#include <optional>

NF_BEGIN_NAMESPACE

namespace po = boost::program_options;

/**
 * @ingroup nf_core_BaseApp
 * @brief The class for setting up and parsing command line options
 *
 * This is helper class for accepting command line options, setting up the options supported,
 * and parse the options values. User of this class must first create an instance of
 * it by passing the incoming command options. Then the supported options must be
 * specified using @ref add() functions. These functions may be called multiple times.
 * After this, user must call @ref parse() to do an initial parsing/validation
 * of the received arguments. Then user may call @ref isSpecified() and
 * @ref value() functions to get the parsed values.
 */
class ApplicationOptions final : boost::noncopyable
{
public:
    /**
     * @brief Constructor.
     * @param argc Count of command line arguments.
     * @param argv List of command line arguments.
     */
    ApplicationOptions(int argc, const char **argv) noexcept;

    /**
     * @brief Function to add argument-less options.
     * @param optionName Name of the option. Eg. "help". User may specify shorthand notation
     * also within the string. Eg. "version,v".
     * @param helpText The text describing the option usage.
     */
    void add(const char *optionName, const char *helpText) noexcept;

    /**
     * @brief Function to add argumented options.
     * @tparam tOptionType Type of the option value.
     */
    template <typename tOptionType>
    void add(const char *optionName, const char *helpText) noexcept
    {
        m_options.add_options()(optionName, po::value<tOptionType>(), helpText);
    }

    /**
     * @brief Function to check if an argument-less option was specified.
     */
    bool isSpecified(const char *optionName) const noexcept;

    /**
     * @brief Function to get the value specified with an option.
     * @return The parsed option value. Returns std::nullopt, if parsing failed.
     */
    template <typename tOptionType>
    std::optional<tOptionType> value(const char *optionName,
                                     const tOptionType &defaultValue) const noexcept
    {
        if (!isSpecified(optionName)) {
            return defaultValue;
        }
        try {
            return m_vm[optionName].as<tOptionType>();
        } catch (...) {
            return std::nullopt;
        }
    }

    /**
     * @brief Parse the command line options.
     *
     * This function parses and validates the command options
     *
     * @return true, if the parsing was successful
     */
    bool parse() noexcept;

    /**
     * @brief Prints the help messages for the application
     */
    void printHelp() const noexcept;

private:
    int m_argc;
    const char **const m_argv;
    po::options_description m_options{"Options"};
    po::variables_map m_vm;
};

NF_END_NAMESPACE
