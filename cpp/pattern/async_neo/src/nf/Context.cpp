/*
 * BMW Neo Framework
 *
 * Copyright (C) 2021-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include "nf/Context.h"

#include <nf/Assert.h>
#include <nf/Executor.h>

using namespace nf;

Context::Context() noexcept
    : Context(Executor::thisThread())
{
}

Context::Context(const std::shared_ptr<Executor> &executor) noexcept
    : m_state(std::make_shared<detail::ContextState>(executor))
{
    assertThat(executor,
               "Context cannot be created with a null executor. Is framework initialized?");
}

void Context::post(Task &&task) const noexcept
{
    m_state->post(std::move(task));
}

void Context::post(Executor::Delay when, Task &&task) const noexcept
{
    m_state->post(when, std::move(task));
}

void Context::bind(Subscription &&sub) const noexcept
{
    m_state->bind(std::move(sub));
}

void Context::bind(RaiiToken &&token) const noexcept
{
    m_state->bind(std::move(token));
}

void Context::bind(std::shared_ptr<ContextItem> item) const noexcept
{
    m_state->bind(std::move(item));
}

bool Context::isEmpty() const noexcept
{
    return m_state->isEmpty();
}

void Context::reset() noexcept
{
    detail::ContextState::reset(m_state);
}

bool Context::isThisThread() const noexcept
{
    return m_state->isThisThread();
}
