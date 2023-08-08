/*
 * BMW Neo Framework
 *
 * Copyright (C) 2021-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include "nf/ContextItem.h"

#include <nf/Assert.h>
#include <nf/Context.h>
#include <nf/detail/ContextState.h>

using namespace nf;

ContextItem::ContextItem() noexcept = default;
ContextItem::~ContextItem() = default;

void ContextItem::contextualize(std::weak_ptr<detail::ContextState> &&state, Iterator it) noexcept
{
    assertThat(m_state.expired(), "Item is already bound to a context");

    m_it = it;
    m_state = std::move(state);
}

void ContextItem::decontextualize() noexcept
{
    if (auto state = m_state.lock()) {
        std::unique_lock lock(state->m_mutex);
        m_state.reset();
        state->m_items.erase(m_it);
    }
}

void ContextItem::post(Task &&task) noexcept
{
    if (auto state = m_state.lock()) {
        state->post([wSelf = weak_from_this(), task = std::move(task)] {
            if (auto self = wSelf.lock()) {
                task();
            }
        });
    }
}

void nf::detail::bind(const Context &context, std::shared_ptr<ContextItem> item) noexcept
{
    context.bind(std::move(item));
}
