/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include "nf/Singleton.h"

#include <nf/Logging.h>

using namespace nf;
using namespace nf::detail;

namespace {

auto &singletonEntries() noexcept
{
    static std::set<SingletonRegistry::Entry> s_singletonEntries;
    return s_singletonEntries;
}

} // anonymous namespace

void SingletonHolderBase::logNoInstance(const std::string &objName) noexcept
{
    nf::warn("An instance of the singleton object {} has been already deleted!", objName);
}

void SingletonRegistry::insert(Entry &&entry) noexcept
{
    singletonEntries().insert(std::move(entry));
}

void SingletonRegistry::releaseAll() noexcept
{
    nf::info("Releasing all singletons from the registry...");
    singletonEntries().clear();
}
