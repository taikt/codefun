/*
 * BMW Neo Framework
 *
 * Copyright (C) 2018-2021 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 *     Alexey Larikov <alexey.larikov@bmw.de>
 */

#pragma once

#include "Printable.h"

namespace std { // NOLINT

// array
template <typename T, std::size_t N>
struct array;

template <typename T, std::size_t N>
std::ostream &operator<<(std::ostream &os, const std::array<T, N> &container) noexcept
{
    return os << nf::detail::exclusivePrintable(container);
}

// pair
template <typename T1, typename T2>
std::ostream &operator<<(std::ostream &os, const std::pair<T1, T2> &container) noexcept
{
    return os << nf::detail::exclusivePrintable(container);
}

// tuple
template <typename... Ts>
std::ostream &operator<<(std::ostream &os, const std::tuple<Ts...> &container) noexcept
{
    return os << nf::detail::exclusivePrintable(container);
}

// vector
template <typename T, typename Allocator>
class vector;

template <typename T, typename Allocator>
std::ostream &operator<<(std::ostream &os, const std::vector<T, Allocator> &container) noexcept
{
    return os << nf::detail::exclusivePrintable(container);
}

// deque
template <typename T, typename Allocator>
class deque;

template <typename T, typename Allocator>
std::ostream &operator<<(std::ostream &os, const std::deque<T, Allocator> &container) noexcept
{
    return os << nf::detail::exclusivePrintable(container);
}

// list
inline namespace __cxx11 {
template <typename T, typename Allocator>
class list;

template <typename T, typename Allocator>
std::ostream &operator<<(std::ostream &os,
                         const std::__cxx11::list<T, Allocator> &container) noexcept
{
    return os << nf::detail::exclusivePrintable(container);
}
} // namespace __cxx11

// set
template <typename Key, typename Compare, typename Allocator>
class set;

template <typename Key, typename Compare, typename Allocator>
std::ostream &operator<<(std::ostream &os,
                         const std::set<Key, Compare, Allocator> &container) noexcept
{
    return os << nf::detail::exclusivePrintable(container);
}

// unordered_set
template <typename Key, typename Hash, typename KeyEqual, typename Allocator>
class unordered_set;

template <typename Key, typename Hash, typename KeyEqual, typename Allocator>
std::ostream &
operator<<(std::ostream &os,
           const std::unordered_set<Key, Hash, KeyEqual, Allocator> &container) noexcept
{
    return os << nf::detail::exclusivePrintable(container);
}

// map
template <typename Key, typename T, typename Compare, typename Allocator>
class map;

template <typename Key, typename T, typename Compare, typename Allocator>
std::ostream &operator<<(std::ostream &os,
                         const std::map<Key, T, Compare, Allocator> &container) noexcept
{
    return os << nf::detail::exclusivePrintable(container);
}

// unordered_map
template <typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator>
class unordered_map;

template <typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator>
std::ostream &
operator<<(std::ostream &os,
           const std::unordered_map<Key, T, Hash, KeyEqual, Allocator> &container) noexcept
{
    return os << nf::detail::exclusivePrintable(container);
}

// multimap
template <typename Key, typename T, typename Compare, typename Allocator>
class multimap;

template <typename Key, typename T, typename Compare, typename Allocator>
std::ostream &operator<<(std::ostream &os,
                         const std::multimap<Key, T, Compare, Allocator> &container) noexcept
{
    return os << nf::detail::exclusivePrintable(container);
}

// unordered_multimap
template <typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator>
class unordered_multimap;

template <typename Key, typename T, typename Hash, typename KeyEqual, typename Allocator>
std::ostream &
operator<<(std::ostream &os,
           const std::unordered_multimap<Key, T, Hash, KeyEqual, Allocator> &container) noexcept
{
    return os << nf::detail::exclusivePrintable(container);
}

} // namespace std
