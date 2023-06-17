/*
 * BMW Neo Framework
 *
 * Copyright (C) 2021-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include "nf/detail/ContextState.h"

#include <nf/Context.h>
#include <nf/Executor.h>
#include <nf/Logging.h>
#include <nf/RaiiToken.h>
#include <nf/Subscription.h>

using namespace nf;
using namespace nf::detail;

ContextState &ContextState::from(const Context &context) noexcept
{
    return *context.m_state;
}

void ContextState::reset(std::shared_ptr<ContextState> &state) noexcept
{
    auto newState = std::make_shared<ContextState>(state->m_executor);
    std::swap(state, newState);
}

ContextState::ContextState(const std::shared_ptr<Executor> &executor) noexcept
    : m_executor(executor)
{
}

ContextState::~ContextState() = default;

void ContextState::post(Task &&task) noexcept
{
    auto storedTask = store(AnyFunction(std::move(task)));
    m_executor->post([storedTask = std::move(storedTask)] { storedTask->invokeSync<Task>(); });
}

void ContextState::post(Executor::Delay when, Task &&task) noexcept
{
    auto storedTask = store(AnyFunction(std::move(task)));
    m_executor->post(when,
                     [storedTask = std::move(storedTask)] { storedTask->invokeSync<Task>(); });
}

void ContextState::bind(Subscription &&sub) noexcept
{
    std::move(sub).contextualize(*this);
}

void ContextState::bind(RaiiToken &&token) noexcept
{
    std::unique_lock lock(m_mutex);
    m_tokens.emplace_back(std::move(token));
}

void ContextState::bind(Item &&item) noexcept
{
    auto *itemPtr = item.get();
    std::unique_lock lock(m_mutex);
    auto it = m_items.insert(end(m_items), std::move(item));
    itemPtr->contextualize(weak_from_this(), it);
}

bool ContextState::isEmpty() const noexcept
{
    std::unique_lock lock(m_mutex);
    return m_tasks.empty() && m_items.empty();
}

bool ContextState::isThisThread() const noexcept
{
    return m_executor == Executor::thisThread();
}

std::shared_ptr<ContextState::StoredTask> ContextState::store(AnyFunction &&task) noexcept
{
    std::unique_lock lock(m_mutex);
    auto it = m_tasks.insert(std::end(m_tasks), std::move(task));
    return std::make_shared<StoredTask>(weak_from_this(), it);
}
