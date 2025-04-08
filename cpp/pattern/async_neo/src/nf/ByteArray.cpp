/*
 * BMW Neo Framework
 *
 * Copyright (C) 2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Kutyavin Nikolay <nikolay.kutyavin@bmw.de>
 *     Holger Kaelberer <holger.kaelberer@bmw.de>
 */
#include "nf/ByteArray.h"

#include <nf/Logging.h>

#include <boost/algorithm/string/predicate.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include <algorithm>
#include <charconv>
#include <stdexcept>

using namespace nf;

// Convert a hex-character into the std::uint8_t number
static std::uint8_t hexToInt(char val) noexcept(false)
{
    if (val >= '0' && val <= '9') {
        return static_cast<std::uint8_t>(val - '0');
    }

    if (val >= 'A' && val <= 'F') {
        return static_cast<std::uint8_t>(val - 'A' + 10);
    }

    if (val >= 'a' && val <= 'f') {
        return static_cast<std::uint8_t>(val - 'a' + 10);
    }

    throw std::invalid_argument("Invalid format for hex-value");
}

// Convert two hex-characters into one byte and skip specialCharacters
static std::byte decodeOneByte(std::string_view::const_iterator &it,
                               std::string_view::const_iterator end,
                               std::string_view specialCharacters)
{
    std::uint8_t res = 0;
    auto isIgnored = [&specialCharacters](char ch) {
        return std::isspace(static_cast<unsigned char>(ch)) != 0
            || specialCharacters.find(ch) != std::string_view::npos;
    };
    for (std::size_t i = 0; i < 2 * sizeof(std::byte); ++i, ++it) {
        while (it != end && isIgnored(*it)) {
            ++it;
        }
        if (it == end) {
            throw std::invalid_argument("Wrong number of symbols in the hex-string");
        }

        res = static_cast<std::uint8_t>((16 * res) + hexToInt(*it));
    }
    return static_cast<std::byte>(res);
}

ByteArray ByteArray::fromString(std::string_view bytes)
{
    ByteArray arr;
    arr.m_data.reserve(bytes.size());
    std::transform(bytes.cbegin(), bytes.cend(), std::back_inserter(arr.m_data),
                   [](auto ch) { return std::byte{static_cast<unsigned char>(ch)}; });
    return arr;
}

nf::Result<ByteArray> ByteArray::fromHex(std::string_view hex) noexcept
{
    return fromHex(hex, "");
}

Result<ByteArray> ByteArray::fromHex(std::string_view hex,
                                     std::string_view specialCharacters) noexcept
{
    try {
        ByteArray arr;
        arr.m_data.reserve(hex.size() / 2);
        const auto *it = hex.cbegin();
        while (it != hex.cend()) {
            arr.m_data.push_back(decodeOneByte(it, hex.cend(), specialCharacters));
        }

        return arr;
    } catch (std::exception &) {
        return nf::result::Err{};
    }
}

Result<ByteArray> ByteArray::fromBase64(std::string_view base64) noexcept
{
    using namespace boost::archive::iterators;
    using FromBase64Iter = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;
    const auto charToByte = [](char ch) -> std::byte { return static_cast<std::byte>(ch); };
    try {
        // Note: binary_from_base64 converts *all* '=' to '\0', therefore need to
        // ... manually check for non-padding '=' and
        // ... trim exactly as many trailing '\0' from the decoded result as '=' were appended
        //     to the Base64 input.
        const auto size = base64.size();
        const auto firstPadding = base64.find('=');
        if (firstPadding != std::string::npos && firstPadding < size - 2) {
            nf::error("Unable to decode Base64 string: Illegal character '=' at index {}",
                      firstPadding);
            return nf::result::Err{};
        }

        const auto sizeWithoutPadding = (firstPadding != std::string::npos) ? firstPadding : size;
        const auto validPadding = (4 - (sizeWithoutPadding % 4)) % 4;
        const auto numPadding = size - sizeWithoutPadding;
        if (numPadding > 0 && numPadding != validPadding) {
            nf::error("Unable to decode Base64 string: Illegal padding of length {}", numPadding);
            return nf::result::Err{};
        };

        auto decoded =
            ByteArray(boost::make_transform_iterator(FromBase64Iter(std::begin(base64)),
                                                     charToByte),
                      boost::make_transform_iterator(FromBase64Iter(std::end(base64)), charToByte));
        return decoded.erase(decoded.size() - numPadding);
    } catch (const std::exception &exc) {
        nf::error("Unable to decode Base64 string: {}", exc.what());
        return nf::result::Err{};
    }
}

ByteArray::ByteArray(std::initializer_list<std::byte> initializer)
    : m_data(initializer)
{
}

const std::byte *ByteArray::data() const noexcept
{
    return m_data.data();
}

std::byte *ByteArray::data() noexcept
{
    return m_data.data();
}

ByteArray::Iterator ByteArray::begin() noexcept
{
    return m_data.begin();
}

ByteArray::Iterator ByteArray::end() noexcept
{
    return m_data.end();
}

ByteArray::ConstIterator ByteArray::cbegin() const noexcept
{
    return m_data.cbegin();
}


ByteArray::ConstIterator ByteArray::cend() const noexcept
{
    return m_data.cend();
}

ByteArray ByteArray::mid(std::size_t index) const noexcept(false)
{
    return mid(index, m_data.size());
}

ByteArray ByteArray::mid(std::size_t index, std::size_t length) const noexcept(false)
{
    ByteArray arr;
    arr.m_data = m_data.substr(index, length);
    return arr;
}

ByteArray ByteArray::first(std::size_t count) const
{
    ByteArray arr;
    arr.m_data = m_data.substr(0, count);
    return arr;
}

ByteArray ByteArray::last(std::size_t count) const
{
    const auto actualCount = std::min(count, m_data.size());
    ByteArray arr;
    arr.m_data = m_data.substr(m_data.size() - actualCount, actualCount);
    return arr;
}

bool ByteArray::startsWith(const ByteArray &other) const noexcept
{
    return boost::starts_with(m_data, other.m_data);
}

bool ByteArray::endsWith(const ByteArray &other) const noexcept
{
    return boost::ends_with(m_data, other.m_data);
}

bool ByteArray::contains(const ByteArray &other) const noexcept
{
    return boost::contains(m_data, other.m_data);
}

bool ByteArray::matches(std::size_t index, const ByteArray &other) const noexcept
{
    if (index >= m_data.size()) {
        return false;
    }

    auto begin = cbegin();
    std::advance(begin, index);

    return boost::starts_with(std::pair{begin, cend()}, std::pair{other.cbegin(), other.cend()});
}

bool ByteArray::operator==(const ByteArray &other) const noexcept
{
    return m_data == other.m_data;
}

bool ByteArray::operator!=(const ByteArray &other) const noexcept
{
    return m_data != other.m_data;
}

std::size_t ByteArray::size() const noexcept
{
    return m_data.size();
}

bool ByteArray::isEmpty() const noexcept
{
    return m_data.empty();
}

std::byte ByteArray::operator[](std::size_t index) const noexcept
{
    return m_data[index];
}

std::byte &ByteArray::operator[](std::size_t index) noexcept
{
    return m_data[index];
}

std::byte ByteArray::at(std::size_t index) const noexcept(false)
{
    return m_data.at(index);
}
std::byte &ByteArray::at(std::size_t index) noexcept(false)
{
    return m_data.at(index);
}

void ByteArray::push_back(std::byte value)
{
    m_data.push_back(value);
}

ByteArray &ByteArray::append(std::byte value)
{
    m_data.append(1, value);
    return *this;
}

ByteArray &ByteArray::append(const ByteArray &other)
{
    m_data.append(other.m_data);
    return *this;
}

ByteArray::Iterator ByteArray::insert(Iterator before, std::byte value)
{
    return m_data.insert(before, value);
}

ByteArray::Iterator ByteArray::insert(ConstIterator before, std::byte value)
{
    return m_data.insert(before, value);
}

ByteArray &ByteArray::insert(std::size_t index, std::byte value) noexcept(false)
{
    m_data.insert(index, 1, value);
    return *this;
}

ByteArray &ByteArray::insert(std::size_t index, const ByteArray &other) noexcept(false)
{
    m_data.insert(index, other.m_data);
    return *this;
}

void ByteArray::popBack() noexcept
{
    m_data.pop_back();
}

void ByteArray::clear() noexcept
{
    m_data.clear();
}

ByteArray &ByteArray::erase(std::size_t index, std::size_t count) noexcept(false)
{
    m_data.erase(index, count);
    return *this;
}

ByteArray &ByteArray::erase(std::size_t index) noexcept(false)
{
    m_data.erase(index, size());
    return *this;
}

ByteArray::Iterator ByteArray::erase(Iterator position) noexcept
{
    return m_data.erase(position);
}

ByteArray::Iterator ByteArray::erase(ConstIterator position) noexcept
{
    return m_data.erase(position);
}

ByteArray::Iterator ByteArray::erase(Iterator begin, Iterator end) noexcept
{
    return m_data.erase(begin, end);
}

ByteArray::Iterator ByteArray::erase(ConstIterator begin, ConstIterator end) noexcept
{
    return m_data.erase(begin, end);
}

void ByteArray::reserve(std::size_t capacity)
{
    m_data.reserve(capacity);
}

std::size_t ByteArray::capacity() const noexcept
{
    return m_data.capacity();
}

void ByteArray::resize(std::size_t size)
{
    m_data.resize(size);
}

void ByteArray::resize(std::size_t size, std::byte value)
{
    m_data.resize(size, value);
}

void ByteArray::swap(ByteArray &other) noexcept
{
    m_data.swap(other.m_data);
}

std::string ByteArray::toBase64() const noexcept
{
    using namespace boost::archive::iterators;
    const auto byteToChar = [](const std::byte &byte) -> char { return static_cast<char>(byte); };

    auto toCharBegin = boost::make_transform_iterator(cbegin(), byteToChar);
    auto toCharEnd = boost::make_transform_iterator(cend(), byteToChar);
    using ToBase64Iter = base64_from_binary<transform_width<decltype(toCharBegin), 6, 8>>;

    auto result = std::string{ToBase64Iter(toCharBegin), ToBase64Iter(toCharEnd)};
    return result.append((3 - m_data.size() % 3) % 3, '=');
}
