/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019-2021 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Thomas Leichtle <thomas.leichtle@bmw.de>
 *     Marko Popović <marko.popovic@bmw.de>
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#pragma once

#include <nf/Global.h>

#include <algorithm>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

NF_BEGIN_NAMESPACE

namespace detail {

template <typename tValue>
using ValOrConstRef = std::conditional_t<sizeof(tValue) <= sizeof(void *), tValue,
                                         std::reference_wrapper<const tValue>>;

template <typename tCompare, typename = void>
inline constexpr bool IsTransparentCompare = false;
template <typename tCompare>
inline constexpr bool
    IsTransparentCompare<tCompare, std::void_t<typename tCompare::is_transparent>> = true;

} // namespace detail

/**
 * @addtogroup nf_core_ContainerAlgorithms
 * @{
 */

/**
 * @brief Check if the @e container contains the object @e val.
 * @param container Container to search for an object.
 * @param value Value to search for in the container.
 * @return true if there is such an element, otherwise false.
 */
template <typename tCon>
bool contains(const tCon &container, const typename tCon::value_type &value) noexcept
{
    return std::find(container.cbegin(), container.cend(), value) != container.cend();
}

/**
 * @brief Check if the @e container contains an element that satisfies a given predicate.
 * @param container Container to search for an object.
 * @param predicate Unary predicate used to search the container.
 * @return true if there is such an element, otherwise false.
 */
template <typename tCon, typename tPred>
bool containsIf(const tCon &container, tPred &&predicate) noexcept
{
    return container.cend()
        != std::find_if(container.cbegin(), container.cend(), std::forward<tPred>(predicate));
}

/**
 * @brief Check if the @e container contains an object with @e key.
 * @param container Container to search for an object.
 * @param key Key of the object to search for.
 * @return true if there is such an element, otherwise false.
 */
template <typename tCon,
          typename = std::enable_if_t<!detail::IsTransparentCompare<typename tCon::key_compare>>>
bool containsKey(const tCon &container, const typename tCon::key_type &key) noexcept
{
    return container.find(key) != container.cend();
}

/* containsKey() overload to support transparent comparators. */
template <typename tCon, typename tKey,
          typename = std::enable_if_t<detail::IsTransparentCompare<typename tCon::key_compare>>>
bool containsKey(const tCon &container, tKey &&key) noexcept
{
    return container.find(std::forward<tKey>(key)) != container.cend();
}

/**
 * @brief Check if the @e container contains the object @e val.
 * @param container Container to search for an object.
 * @param value Value to search for in the container.
 * @return true if there is such an element, otherwise false.
 */
template <typename tCon>
bool containsValue(const tCon &container, const typename tCon::mapped_type &value) noexcept
{
    return containsIf(container,
                      [&](const typename tCon::value_type &pair) { return pair.second == value; });
}

/**
 * @brief Find an element in the @e container that equals a given @p value.
 * @param container Container to search for an object.
 * @param value Value used to search the container.
 * @return Element that matches a given value, if one exists. Otherwise @e std::nullopt.
 *
 * Same rules as for iterators apply: The returned object is invalidated and must not be accessed
 * when the container is modified or deleted.
 */
template <typename tCon>
auto find(const tCon &container, const typename tCon::value_type &value) noexcept
{
    const auto it = std::find(container.begin(), container.end(), value);
    return it == container.cend() ?
        std::nullopt :
        std::optional<detail::ValOrConstRef<typename tCon::value_type>>(*it);
}

/**
 * @brief Find an element in the @e container that equals a given @p value.
 * @param container Container to search for an object.
 * @param value Value used to search the container.
 * @return Non-const reference to a matching element, if one exists. Otherwise @e std::nullopt.
 *
 * Same rules as for iterators apply: The returned object is invalidated and must not be accessed
 * when the container is modified or deleted.
 */
template <typename tCon>
auto find(tCon &container, const typename tCon::value_type &value) noexcept
{
    const auto it = std::find(container.begin(), container.end(), value);
    return it == container.cend() ?
        std::nullopt :
        std::optional<std::reference_wrapper<typename tCon::value_type>>(*it);
}

/**
 * @brief Find an element in the @e container that satisfies a given @p predicate.
 * @param container Container to search for an object.
 * @param predicate Unary predicate used to search the container.
 * @return Element that satisfies @p predicate, if it exists. Otherwise @e std::nullopt.
 *
 * Same rules as for iterators apply: The returned object is invalidated and must not be accessed
 * when the container is modified or deleted.
 */
template <typename tCon, typename tPred>
auto findIf(const tCon &container, tPred &&predicate) noexcept
{
    const auto it =
        std::find_if(container.begin(), container.end(), std::forward<tPred>(predicate));
    return it == container.cend() ?
        std::nullopt :
        std::optional<detail::ValOrConstRef<typename tCon::value_type>>(*it);
}

/**
 * @brief Find an element in the @e container that satisfies a given @p predicate.
 * @param container Container to search for an object.
 * @param predicate Unary predicate used to search the container.
 * @return Non-const reference to a matching element, if one exists. Otherwise @e std::nullopt.
 *
 * Same rules as for iterators apply: The returned object is invalidated and must not be accessed
 * when the container is modified or deleted.
 */
template <typename tCon, typename tPred>
auto findIf(tCon &container, tPred &&predicate) noexcept
{
    const auto it =
        std::find_if(container.begin(), container.end(), std::forward<tPred>(predicate));
    return it == container.cend() ?
        std::nullopt :
        std::optional<std::reference_wrapper<typename tCon::value_type>>(*it);
}

/**
 * @brief Find an object in the @e container by @e key.
 * @param container Container to search for an object.
 * @param key Key of the object to search for.
 * @return Object with @e key, if it exists in @e container - otherwise std::nullopt.
 *
 * Same rules as for iterators apply: The returned object is invalidated and must not be accessed
 * when the container is modified or deleted.
 */
template <typename tCon>
auto findByKey(const tCon &container, const typename tCon::key_type &key) noexcept
{
    const auto it = container.find(key);
    return it == container.cend() ?
        std::nullopt :
        std::optional<detail::ValOrConstRef<typename tCon::mapped_type>>(it->second);
}

/**
 * @brief Find an object in the @e container and get a non-const reference.
 * @param container Container to search for an object.
 * @param key Key of the object to search for.
 * @return Reference to object with @e key, if it exists in @e container - otherwise std::nullopt.
 *
 * Same rules as for iterators apply: The returned object is invalidated and must not be accessed
 * when the container is modified or deleted.
 */
template <typename tCon>
auto findByKey(tCon &container, const typename tCon::key_type &key) noexcept
{
    const auto it = container.find(key);
    return it == container.end() ?
        std::nullopt :
        std::optional<std::reference_wrapper<typename tCon::mapped_type>>(it->second);
}

/**
 * @brief Transform any container into a vector.
 * @param container Container to transform.
 * @param functor Functor for transformation.
 * @return std::vector containing the transformed elements.
 */
template <typename tCon, typename tFunctor>
auto transformToVector(const tCon &container, tFunctor &&functor) noexcept
{
    std::vector<typename std::result_of<tFunctor(typename tCon::value_type)>::type> result;
    result.reserve(container.size());
    std::transform(std::cbegin(container), std::cend(container), std::back_inserter(result),
                   std::forward<tFunctor>(functor));
    return result;
}

/**
 * @brief Copy elements of one container which match the predicate to another one.
 * @param iterOut Output iterator.
 * @param conIn Input container.
 * @param pred A predicate to match elements from @p conIn.
 *
 * This is a wrapper around @cppdoc{std::copy_if,algorithm/copy} which works with ranges.
 *
 * @since 2.5
 */
template <typename tConIn, typename tIterOut, typename tPred>
auto copyIf(const tConIn &conIn, tIterOut &&iterOut, tPred &&pred) noexcept
{
    return std::copy_if(std::cbegin(conIn), std::cend(conIn), std::forward<tIterOut>(iterOut),
                        std::forward<tPred>(pred));
}

/**
 * @brief Append elements of one container which match the predicate to another one.
 * @param conOut Output container. It must have @c std::back_inserter().
 * @param conIn Input container.
 * @param pred A predicate to match elements from @p conIn.
 *
 * This function appends elements of @p conIn which match the provided @p pred to @p conOut.
 * The predicate is a callable which accepts an element of @p conIn and returns @e true if
 * this element must be appended to @p conOut.
 *
 * @since 2.5
 */
template <typename tConIn, typename tConOut, typename tPred>
auto insertBackIf(const tConIn &conIn, tConOut &conOut, tPred &&pred) noexcept
{
    return copyIf(conIn, std::back_inserter(conOut), std::forward<tPred>(pred));
}

/**
 * @brief Append elements of one container which match the predicate to another one.
 * @param conIn Input container.
 * @param conOut Output container. It must have @c std::inserter().
 * @param pred A predicate to match elements from @p conIn.
 *
 * This function appends elements of @p conIn which match the provided @p pred to @p conOut.
 * The predicate is a callable which accepts an element of @p conIn and returns @e true if
 * this element must be appended to @p conOut.
 */
template <typename tConIn, typename tConOut, typename tPred>
auto insertIf(const tConIn &conIn, tConOut &conOut, tPred &&pred) noexcept
{
    return copyIf(conIn, std::inserter(conOut, std::end(conOut)), std::forward<tPred>(pred));
}

/**
 * @brief Erase elements from a container that satisfy a given predicate.
 * @param container Container which will be modified.
 * @param predicate Predicate used to select elements which will be erased.
 *
 * This function implements the remove-erase idiom for non-associative containers.
 *
 * @since 2.10
 */
template <typename tCon, typename tPred>
auto eraseIf(tCon &container, const tPred &predicate)
{
    auto it = std::remove_if(std::begin(container), std::end(container), predicate);
    return container.erase(it, std::end(container));
}

/**
 * @brief Remove the top/front element from a container and return it.
 *
 * This function removes the top/front element from a container and returns it. The provided
 * container must have these operations defined:
 * - @c top() or @c front()
 * - @c pop()
 *
 * Standard @cppdoc{queue,container/queue}, @cppdoc{priority_queue,container/priority_queue}
 * and @cppdoc{stack,container/stack} meet these requirements.
 *
 * @warning Calling this method on an empty container causes undefined behavior.
 *
 * @par Example
 * @code
 * std::queue<int> queue{{1, 2, 3, 4, 5}};
 * int first = nf::pop(queue); // first == 1, queue == {2, 3, 4, 5}
 * @endcode
 *
 * @since 3.11
 */
template <typename tCon>
auto pop(tCon &container) noexcept -> std::remove_reference_t<decltype(container.front())>
{
    auto entry = std::move(container.front());
    container.pop();
    return entry;
}

template <typename tCon>
auto pop(tCon &container) noexcept
    -> std::remove_cv_t<std::remove_reference_t<decltype(container.top())>>
{
    auto entry = container.top();
    container.pop();
    return entry;
}

/// @} // nf_core_ContainerAlgorithms

NF_END_NAMESPACE
