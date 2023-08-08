/*
 * BMW Neo Framework
 *
 * Copyright (C) 2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Nikolai Kutiavin <nikolai.kutiavin@bmw.de>
 *     Holger Kaelberer <holger.kaelberer@bmw.de>
 */

#pragma once

#include <nf/Result.h>

#include <cstddef>
#include <cstring>
#include <string>
#include <type_traits>

NF_BEGIN_NAMESPACE

/**
 * @ingroup nf_core_Types
 * @brief @c ByteArray is a storage of bytes and a set of functions for a manipulation with that
 * storage.
 *
 * You can use in a code <code>std::vector<std::byte> </code> or
 * <code> std::basic_string<std::byte> </code> to replace @c ByteArray and implement all required
 * methods by your own, but this class offers some futures that makes coding easy.
 *
 * Consider that we have a wav-file and want to read that from the disk, validate and get
 * some parameters.
 *
 * @note Information about the wav-format can be read from here
 * http://soundfile.sapp.org/doc/WaveFormat/.
 *
 * Next example demonstrates how a file can be read into a @c ByteArray with @b STL algorithm and
 * @c std::back_inserter. A preallocation of memory for the @c ByteArray is used here to
 * accelerate runtime execution.
 *
 * @code {.cpp}
 * ByteArray readFromFile(std::string_view file) noexcept(false)
 * {
 *   std::ifstream inBinaryFile(file.data(), std::ios::binary | std::ios::in);
 *
 *   if (!inBinaryFile.is_open()) {
 *       throw std::runtime_error("Failed to open file");
 *   }
 *
 *   ByteArray result;
 *   result.reserve(std::filesystem::file_size(file)); // preallocate space for the whole file
 *   auto cast = [](char c) {
              return static_cast<std::byte>(c);
 *         };
 *
 *   // next call reads bytes from the file and pushes that back into the `ByteArray`
 *   // transform is used here because there is no implementation
 *   // for std::istreambuf_iterator<std::byte>
 *   std::transform(std::istreambuf_iterator<char>(inBinaryFile),
 *                  std::istreambuf_iterator<char>{},
 *                  std::back_inserter(result), cast);
 *   return result;
 * }
 * @endcode
 *
 * By the wav-format specification a file must start with the fixed prefix and
 * bytes from 8th till 12th express the format of the file and have fixed values. Below
 * variables are created for the mentioned data:
 *
 * @code
 * using namespace nf::bytearray_literals; // enable _hex literal
 *
 *  // any user code
 *
 * // Make validation of hex-string in runtime
 * nf::Result<ByteArray> prefix = ByteArray::fromHex("52 49 46 46");
 *
 * // Make validation of hex-number in compile time
 * ByteArray expectedFormat = 0x57'41'56'45_hex;
 * @endcode
 *
 * Now everything is ready for the checks:
 *
 * @code
 * ByteArray wavContent = readFromFile("my-file.wav");
 * if (wavContent.startsWith(prefix) && wavContent.matches(8, expectedFormat)){
 *   // we have the appropriative wav-file
 * } else {
 *   std::cerr << "The wav-file has a brocken format" << std::endl;
 * }
 * @endcode
 *
 * For some additional checks `STL` or `boost` algorithms can be used:
 *
 * @code
 * // Get an iterator to a byte with value `0xFF` or `0xA1`
 * auto it = std::find_if(wavContent.cbegin(), wavContent.cend(),
 *                        [](std::byte bt){
                               return bt == std::byte{0xFF} || bt == std::byte{0xA1};
                            });
 *
 * // Test if the `wavContent` ends with a byte sequence represented with `vector`
 * auto check = std::vector<std::byte>{std::byte{0x00}, std::byte{0x00}, std::byte{0xff}};
 * if(boost::ends_with(std::pair{wavContent.cbegin(), wavContent.cend()}, check)) {
 *    // any user code
 * }
 * @endcode
 *
 * At this point the @c ByteArray is created and validated and some data can be read from that.
 * To simplify the example, let's consider that current system is @b little endian.
 *
 * Let's say, we want to retrieve some information from the wav-file and save it in the structure
 * presented below. Supported formats are listed as @c enum:
 *
 * @code
 * enum class AudioFormat : uint16_t
 * {
 *   Pcm = 1,
 *   IeeeFloat = 3,
 *   Mpeg = 80,
 *   Wma = 353
 * };
 *
 * struct SubChunk1Header
 * {
 *   uint32_t size;
 *   AudioFormat format;
 *   uint16_t numChanels;
 * };
 * @endcode
 *
 * Further lines of code present how read can be done for the numbers. Note that there
 * @b sized integers are used.
 *
 * ```
 * SubChunk1Header header;
 * // Read size of the chunk started from the 16th bit and encoded into 4 bits.
 * header.size = wavContent.reinterpret<uint32_t>(16).value_or(0);
 * // Read count of channels started from the 22th bit and encoded into 2 bits
 * header.numChanels = wavContent.reinterpret<uint16_t>(22).value_or(0);
 * ```
 *
 * Next is reading an @c AudioFormat value. It is a bit complicated, because a random wav-file can
 * contain any value, but the goal here is reading only few of them and throw an exception for the
 * others.
 *
 * @code
 * // Read audio format started from 20th bit and encoded into 2 bits
 * auto format = static_cast<AudioFormat>(wavContent.reinterpret<uint16_t>(20).value_or(0));
 *
 * // Test if format has a defined value
 * switch (format) {
 *   case AudioFormat::Pcm:
 *     [[fallthrough]];
 *   case AudioFormat::IeeeFloat:
 *     [[fallthrough]];
 *   case AudioFormat::Mpeg:
 *     [[fallthrough]];
 *   case AudioFormat::Wma:
 *     header.format = format;
 *     break;
 *   default:
 *     // audio format does not matched to expected values, so throw the exception
 *     throw std::runtime_error("The audio format is wrong");
 * }
 * @endcode
 *
 * Let's consider following example of `ByteArray` creation and writing data into it:
 *
 * @code
 * ByteArray wavFile;
 *
 * // Write header for the chunk
 * wavFile
 * .append(ByteArray::fromString("RIFF")) // add ChunkID
 * .insertAsBytes(wavFile.size(), static_cast<uint32_t>(36)) // Size for file without audio data
 * .append(0x57415645_hex);               // Add format for file
 *
 * // Write sub-chunk 1 ID with `std::back_inserter`
 * const auto subChunk1Id = std::string_view("fmt ");
 * std::transform(subChunk1Id.cbegin(), subChunk1Id.cend(), std::back_inserter(wavFile),
 *                [](char c){return static_cast<std::byte>(c);});
 *
 * // any user code
 *
 * //Write the content of the `ByteArray` into the file:
 * std::ofstream out("/build/out.wav");
 * std::transform(wavFile.cbegin(), wavFile.cend(), std::ostream_iterator<char>(out),
 *            [](std::byte bt) { return static_cast<char>(bt); });
 * @endcode
 */
class ByteArray
{
public:
    using Container = std::basic_string<std::byte>;
    using ConstIterator = Container::const_iterator;
    using Iterator = Container::iterator;
    // Needed for STL compatibility
    using iterator = Container::iterator;
    using value_type = Container::value_type;

public:
    /**
     * @brief Create a new @p ByteArray from a string.
     *
     * The method interprets the content of the string as a sequence of bytes and creates a new
     * object with those bytes.
     *
     * @code
     * std::string storedContent = "1234";
     * ByteArray arr = ByteArray::fromString(storedContent);
     * // arr content - {0x31, 0x32, 0x33, 0x34}
     * @endcode
     *
     * @param bytes Bytes to store.
     * @return A newly created byte array.
     */
    [[nodiscard]] static ByteArray fromString(std::string_view bytes);

    /**
     * @brief Create a new object from a hex-string while ignoring whitespaces.
     *
     * This function constructs a @p ByteArray out of a @e Base16 encoded string. If this string
     * contains illegal characters, an error is returned. The function is case-insensitive, thus
     * strings like @e "ABAC" and @e "abac" are both valid. Whitespaces is detected by
     * @c std::isspace.
     *
     * @code
     * std::string storedContent = "0a 2B 3C 4f 98 12";
     * auto arr = ByteArray::fromHex(storedContent);
     * // expected content for arr - {0x0A, 0x2B, 0x3C, 0x4F, 0x98, 0x12};
     * if(arr.hasError()) {
     *   <report error>
     * }
     * else {
     *   <handle arr>
     * }
     * @endcode
     *
     * @param hex Hex-encoded string.
     * @return @ok Constructed object.
     * @return @err Default error with type @p nf::result::Err.
     */
    [[nodiscard]] static Result<ByteArray> fromHex(std::string_view hex) noexcept;


    /**
     * @brief Create a new object from a Base64-encoded string.
     *
     * The provided Base64 data is decoded using the algorithm from
     * <a href="http://www.ietf.org/rfc/rfc4648.txt">RFC 4648</a>. It accepts the regular
     * Base64 alphabet only (not the alternate URL friendly one) and fails on any non-alphabet
     * character (including lines feeds, whitespaces and the like).
     *
     * @code
     * std::string base64String = "Zm9vYmFyMQ==";
     * auto arr = ByteArray::fromBase64(base64String);
     * if(arr.hasError()) {
     *   <report error>
     * }
     * else {
     *   <handle arr>
     * }
     * @endcode
     *
     * @param base64 Base64-encoded string.
     * @return @ok A new @ref ByteArray instance.
     * @return @err Default error with type @p nf::result::Err.
     *
     * @since 5.6
     */
    [[nodiscard]] static Result<ByteArray> fromBase64(std::string_view base64) noexcept;


    /**
     * @brief Create a new object from a hex-string while ignoring special characters and
     * whitespaces.
     *
     * The method decodes a @e Base16 encoded hex-string with special characters into a sequence of
     * bytes, skips any symbol from the @p specialCharacters parameter and whitespaces. It means
     * that a @c ByteArray object can be crated from the string "AA AA CC" or even from that one "0a
     * 2B 3C 4f+98*12" with the @p specialCharacters equal to "+*". The function is
     * case-insensitive for the @p hex parameter, thus strings like @e "ABAC" and @e "abac" are both
     * valid. Whitespaces is detected by @c std::isspace.
     *
     * @code
     * std::string storedContent = "0a 2B 3C 4f+98*12";
     * auto arr = ByteArray::fromHex(storedContent, " +*");
     * if(arr.hasError()) {
     *   <report error>
     * }
     * else {
     *   <handle arr>
     * }
     * @endcode
     *
     * @param hex Hex-encoded string.
     * @param specialCharacters List of symbols, that should be ignored during decoding.
     * @return @ok Constructed object.
     * @return @err Default error with type @p nf::result::Err.
     */
    [[nodiscard]] static Result<ByteArray> fromHex(std::string_view hex,
                                                   std::string_view specialCharacters) noexcept;

public:
    ByteArray() noexcept = default;
    ByteArray(const ByteArray &other) = default;
    ByteArray(ByteArray &&other) noexcept = default;
    ~ByteArray() = default;
    ByteArray &operator=(const ByteArray &other) = default;
    ByteArray &operator=(ByteArray &&other) noexcept = default;

    /**
     * @brief Construct a new @p ByteArray object from the range [begin, end).
     *
     * @tparam tInputIt Satisfies @e LegacyInputIterator and has to point to a type implicitly
     *                  convertible to the @c std::byte.
     * @param begin Start of the input range.
     * @param end End of the input range.
     *
     * @code
     * std::vector<std::byte> content = {std::byte{0x0A}, std::byte{0x2B}, std::byte{0x3C},
     *                                   std::byte{0x4F}, std::byte{0x98}, std::byte{0x12}};
     * ByteArray arr(content.cbegin(), content.cend());
     * @endcode
     */
    template <typename tInputIt>
    ByteArray(tInputIt begin, tInputIt end);

    /**
     * @brief Construct a new @c ByteArray object from initializer list.
     *
     * @code
     * auto arr = ByteArray{std::byte{0x12}, std::byte{0x23}, std::byte{0x09}};
     * @endcode
     *
     * @param initializer List of values that is copied into constructed @c ByteArray.
     */
    ByteArray(std::initializer_list<std::byte> initializer);

public:
    /**
     * @brief Return a read-only pointer to the stored bytes.
     *
     * Returns a pointer to the data inside this container. The pointer dereferences to a contiguous
     * memory region. The data can't be changed with that pointer.
     *
     * @return A read-only pointer to the bytes in the container.
     */
    [[nodiscard]] const std::byte *data() const noexcept;

    /**
     * @brief Return a pointer to the stored bytes.
     *
     * Returns a pointer to the data inside this container. The pointer dereferences to a contiguous
     * memory region.
     *
     * @return Pointer to the bytes in this container.
     */
    [[nodiscard]] std::byte *data() noexcept;

    /**
     * @brief Return an iterator that points to the begin of the array.
     */
    [[nodiscard]] Iterator begin() noexcept;

    /**
     * @brief Return an iterator that points to the end of the array.
     */
    [[nodiscard]] Iterator end() noexcept;

    /**
     * @brief Constant version of @ref ByteArray::begin().
     */
    [[nodiscard]] ConstIterator cbegin() const noexcept;

    /**
     * @brief Constant version of @ref ByteArray::end().
     */
    [[nodiscard]] ConstIterator cend() const noexcept;

    /**
     * @brief Get a sub-array from the @c index till the end.
     *
     * Returns a sub-array that contains elements from the @c index till the end. If the @c index
     * is out of the range <code> [0, size()) </code>, an exception is thrown.
     *
     * @param index Start of the sub-array.
     * @return New object, that contains elements from the old one in range
     *         <code> [index, size()) </code>.
     * @exception std::out_of_range if <code> index > size() </code>.
     */
    [[nodiscard]] ByteArray mid(std::size_t index) const noexcept(false);

    /**
     * @brief Get a sub-array from the @c index and with the size @c length.
     *
     * Returns a new sub-array, that contains elements <code> [index, index + length) </code> from
     * the original one. If <code> index + length </code> is greater than the size of the array, the
     * method brings back elements from the @c index till the end of the container. An exception is
     * thrown if the @c index is out of the range <code> [0, size()) </code>.
     *
     * @param index Start of the sub-array to return.
     * @param length Number of the elements to copy.
     * @return New object,  that contains elements from the old one in the range
     *         <code> [index, index + length) </code> or <code> [index, size()) </code>.
     * @exception std::out_of_range if <code> index > size() </code>.
     */
    [[nodiscard]] ByteArray mid(std::size_t index, std::size_t length) const noexcept(false);

    /**
     * @brief Get a sub-array with the @c count bytes from the beginning of the original one.
     *
     * If <code> count > size() </code> then a copy of the original object is returned back.
     *
     * @code
     * auto arr = ByteArray::fromHex("00 11 AA", " ");
     * auto first = arr.first(2); // first = {0x00, 0x11}
     * @endcode
     *
     * @param count The number of bytes from beginning to copy.
     * @return A sub-array, contained up to the @c count bytes from the beginning of the original
     *         object.
     */
    [[nodiscard]] ByteArray first(std::size_t count) const;

    /**
     * @brief Get a sub-array with the @c count bytes from the end of the @c ByteArray.
     *
     * If <code> count > size() </code> then a copy of the original object is returned back.
     *
     * @code
     * auto arr = ByteArray::fromHex("00 11 AA", " ");
     * auto last = arr.last(2); // last = {0x11, 0xAA}
     * @endcode
     *
     * @param count The number of bytes to copy from the end.
     * @return A sub-array, contained up to the @c count bytes from the end of the original object.
     */
    [[nodiscard]] ByteArray last(std::size_t count) const;

    /**
     * @brief Test if the current object has the prefix @c other.
     *
     * @param other Object to test.
     * @return @c true if @c this starts with @c other and the size of @c this is greater or
     *         equal to the size of @c other, or @c other is empty, @c false otherwise.
     */
    [[nodiscard]] bool startsWith(const ByteArray &other) const noexcept;

    /**
     * @brief Test if the current object ends with bytes from the @c other.
     *
     * @param other Object to test.
     * @return @c true if @c this ends with the @c other and the size of @c this is greater or
     *         equal to the size of the @c other, @c false otherwise.
     */
    [[nodiscard]] bool endsWith(const ByteArray &other) const noexcept;

    /**
     * @brief Test if the current object entirely contains the @c other.
     *
     * @param other Object to test.
     * @return @c true if current object entirely contains the @c other, @c false otherwise.
     */
    [[nodiscard]] bool contains(const ByteArray &other) const noexcept;

    /**
     * @brief Test if the content of @c this starting from the @c index fully matches the @c other.
     *
     * @code
     * auto original = ByteArray::fromHex("00 11 22 33 44 55");
     * auto test = ByteArray::fromHex("33 44");
     * const auto matched = original.matches(3, test); // matched is true
     * @endcode
     *
     * @param index Index of the current object from that the match is checked.
     * @param other Object to test.
     * @return @c true if content of @c this object matches to the @c other, @c false otherwise,
     *         even if the @c index is wrong.
     */
    [[nodiscard]] bool matches(std::size_t index, const ByteArray &other) const noexcept;

    /**
     * @brief Check if two object have the equal content.
     *
     * @param other Object to compare with.
     * @return @c true if content is equal, @c false otherwise.
     */
    [[nodiscard]] bool operator==(const ByteArray &other) const noexcept;

    /**
     * @brief Check if two object have different content.
     *
     * @param other Object to test.
     * @return  @c true if content is not equal, @c false otherwise.
     */
    [[nodiscard]] bool operator!=(const ByteArray &other) const noexcept;

    /**
     * @brief Get the size of the @c ByteArray.
     *
     * @return Number of elements in the container.
     */
    [[nodiscard]] std::size_t size() const noexcept;

    /**
     * @brief Check if the container is empty.
     *
     * @return @c true if the container is empty, @c false otherwise.
     */
    [[nodiscard]] bool isEmpty() const noexcept;

    /**
     * @brief Return the value of the byte in the position @c index.
     *
     * If <code> index >= size() </code>, the behavior is undefined.
     *
     * @param index Position of the byte to return.
     * @return Value of the byte in the requested position.
     */
    [[nodiscard]] std::byte operator[](std::size_t index) const noexcept;

    /**
     * @brief Return the modifiable reference to the byte in the position @c index.
     *
     * If <code> index >= size() </code> the behavior is undefined.
     *
     * @code
     * void assignValueFor10thElement(nf::ByteArray &arr, std::byte bt) {
     *     if(arr.size() > 10) {
     *        arr[10] = bt;
     *     }
     * }
     * @endcode
     *
     * @param index Position of the reference to return.
     * @return Non-const reference to the byte.
     */
    [[nodiscard]] std::byte &operator[](std::size_t index) noexcept;

    /**
     * @brief Return the byte in the postion @c index.
     *
     * @param index Position for the byte to retrieve.
     * @return Value of the byte in the requested position.
     * @exception std::out_of_range if <code> index >= size() </code>.
     */
    [[nodiscard]] std::byte at(std::size_t index) const noexcept(false);

    /**
     * @brief Return the modifiable reference to the byte in the postion @c index.
     *
     * @code
     * void assignValueFor10thElement(nf::ByteArray &arr, std::byte bt) {
     *   try {
     *        arr.at(10) = bt;
     *   } catch(std::out_of_range &) {
     *       std::cerr << "Failed assign value for 10th element" << std::endl;
     *   }
     * }
     * @endcode
     *
     * @param index Position of the byte to retrieve.
     * @return Non-const reference to the byte in position @c index.
     * @exception std::out_of_range if <code> index >= size() </code>.
     */
    [[nodiscard]] std::byte &at(std::size_t index) noexcept(false);

    /**
     * @brief Append a @c value to the end of the @c ByteArray.
     *
     * @note This function @b might trigger memory reallocation and active iterators invalidation.
     *
     * @note This method makes possible using @c ByteArray with @c std::back_inserter.
     *
     * @param value Value to append to the end of the container.
     */
    // NOLINTNEXTLINE(readability-identifier-naming)
    void push_back(std::byte value);

    /**
     * @brief Append a @c value to the end of the @c ByteArray.
     *
     * This function returns a reference to the @c ByteArray, thus it helps with methods chaining.
     *
     * @note This function @b might trigger memory reallocation and active iterators invalidation.
     *
     * @param value Value to append to the end of the container.
     * @return Reference to @c this.
     */
    ByteArray &append(std::byte value);

    /**
     * @brief Append a content of the @c other to the end of the @c ByteArray.
     *
     * This function returns a reference to the @c ByteArray, thus it helps with a methods chaining.
     *
     * @note This function @b might trigger memory reallocation and active iterators invalidation.
     *
     * @param other Object that content should be added to the end of @c this.
     * @return Reference to @c this.
     */
    ByteArray &append(const ByteArray &other);

    /**
     * @brief Insert @c value before the position provided by @c before.
     *
     * This function inserts @c value and returns an iterator that points to the inserted value
     * position.
     *
     * @note This function @b triggers memory reallocation and active iterators invalidation.
     *
     * @code
     * ByteArray arr;
     * arr.insert(arr.begin(), std::byte{0x12}); // arr = {0x12}.
     * arr.insert(arr.begin(), std::byte{0xAA}); // arr = {0xAA, 0x12}.
     * arr.insert(arr.end(), std::byte{0xBB}); // arr = {0xAA, 0x12, 0xBB}.
     * auto secondElement = std::next(arr.begin()); // move iterator to the second element 0x12.
     * arr.insert(secondElement, std::byte{0xCC}); // arr = {0xAA, 0xCC, 0x12, 0xBB}.
     * @endcode
     *
     * @note This method makes possible using @c ByteArray with @c std::inserter.
     *
     * @param before Points to the position before that the value should be inserted.
     * @param value Inserted value.
     * @return Iterator to the inserted value.
     */
    Iterator insert(Iterator before, std::byte value);

    /**
     * @brief Insert @c value before the position provided by the @e constant iterator @c before.
     *
     * @note This function @b triggers memory reallocation and active iterators invalidation.
     *
     * This function accepts a @e constant iterator and offers the same functionality as
     * @ref ByteArray::insert(Iterator, std::byte).
     */
    Iterator insert(ConstIterator before, std::byte value);

    /**
     * @brief Insert @c value into the position provided by @c index.
     *
     * This function returns a reference to the @c ByteArray, thus it helps with a methods chaining.
     * If <code> index > size() </code>, then an exception is thrown.
     *
     * @note This function @b triggers memory reallocation and active iterators invalidation.
     *
     * @code
     * ByteArray arr;
     * arr.insert(0, std::byte{0x11}) // arr = {0x11}
     *    .insert(1, std::byte{0xAA}) // arr = {0x11, 0xAA}
     *    .insert(1, std::byte{0xBB}); // arr = {0x11, 0xBB, 0xAA}
     * @endcode
     *
     * @param index Position for the element insertion, legal values are in the range
     *        <code> [0, size()] </code>.
     * @param value Value to insert.
     * @return Reference to @c this.
     * @exception std::out_of_range if <code> index > size() </code>.
     */
    ByteArray &insert(std::size_t index, std::byte value) noexcept(false);

    /**
     * @brief Insert content of the @c other into the position provided by @c index.
     *
     * This function returns a reference to the @c ByteArray, thus it helps with a methods chaining.
     * If <code> index > size() </code>, then an exception is thrown.
     *
     * @param index Position for the element insertion, legal values are in the range
              <code> [0, size()] </code>.
     * @param other Container that data is copied.
     * @return Reference to @c this.
     * @exception std::out_of_range if <code> index > size() </code>.
     */
    ByteArray &insert(std::size_t index, const ByteArray &other) noexcept(false);

    /**
     * @brief Remove the last element from the @c ByteArray.
     *
     * @note Calling this function for an empty @c ByteArray has undefined behavior. This function
     * invalidates references and iterators to the last element, as well as @ref end() and
     * @ref cend() results.
     */
    void popBack() noexcept;

    /**
     * @brief Remove all elements from the @c ByteArray.
     *
     * @note This function invalidates @b all references and iterators.
     */
    void clear() noexcept;

    /**
     * @brief Remove @c count elements starting from the position @c index.
     *
     * Check if @c index in the range <code> [0, size()) </code> and throw an exception otherwise.
     * If <code> index + count > size() </code> then all elements from @c index till the end of an
     * @c ByteArray are removed.
     *
     * @note This function invalidates references and iterators in the range
     *       <code> [index, size()) </code>.
     *
     * @param index Starting position for the removal.
     * @param count Number of elements for the erasing.
     * @return Reference to @c this.
     * @exception  std::out_of_range if <code> index > size() </code>.
     */
    ByteArray &erase(std::size_t index, std::size_t count) noexcept(false);

    /**
     * @brief Remove elements from the position @c index till the end.
     *
     * @note This function invalidates references and iterators in the range
     *       <code> [index, size()) </code>.
     *
     * @param index Starting position for the removal.
     * @return Reference to @c this.
     * @exception std::out_of_range if <code> index > size() </code>.
     */
    ByteArray &erase(std::size_t index) noexcept(false);

    /**
     * @brief Remove an element in the @c position.
     *
     * @note This function invalidates iterators and references in the range
     *       <code> [position, end()] </code>.
     *
     * @param position Iterator pointing to a single element to be removed from the @c ByteArray.
     * @return An iterator following the removed element.
     */
    Iterator erase(Iterator position) noexcept;

    /**
     * @brief Remove an element in the @c position.
     *
     * This is a constant version of the @ref ByteArray::erase(Iterator).
     */
    Iterator erase(ConstIterator position) noexcept;

    /**
     * @brief Remove elements in the range  <code> [begin, end) </code>.
     *
     * @note This function invalidates iterators and references in the range
     *       <code> [begin, end()] </code>.
     *
     * @param begin Point to the first element in the range to remove.
     * @param end Point to the element till that the removal is done, this element is excluded from
     *        the range.
     * @return An iterator following the last removed element.
     */
    Iterator erase(Iterator begin, Iterator end) noexcept;

    /**
     * @brief  Remove elements in the range <code> [begin, end) </code>.
     *
     * This is a constant version of the @ref ByteArray::erase(Iterator, Iterator).
     */
    Iterator erase(ConstIterator begin, ConstIterator end) noexcept;

    /**
     * @brief Preallocate additional internal space for the container.
     *
     * If @c capacity is bigger then a size of an internal allocated space, then a new storage is
     * allocated. Otherwise this function has no effects.
     *
     * @note This function might invalidate iterators and references.
     *
     * @param capacity New size of an internal storage.
     */
    void reserve(std::size_t capacity);

    /**
     * @brief Return the size of internally allocated storage.
     */
    [[nodiscard]] std::size_t capacity() const noexcept;

    /**
     * @brief Change the size of @c ByteArray.
     *
     * If current size of the container is bigger than @c size, then this method remains only
     * @c size count elements and discards others. Otherwise new elements are added and initialized
     * with @c 0.
     *
     * @note This function might invalidate iterators and references.
     *
     * @param size New size of the container.
     */
    void resize(std::size_t size);

    /**
     * @brief Change the size of @c ByteArray.
     *
     * If current size of the container is bigger than @c size, then this method remains only \
     * @c size count elements and discards others. Otherwise new elements are added and initialized
     * with @c value.
     *
     * @note This function might invalidate iterators and references.
     *
     * @param size New size of the container.
     * @param value Initial value for newly added elements.
     */
    void resize(std::size_t size, std::byte value);

    /**
     * @brief Exchanges the contents of the object with those of other.
     *
     * @param other Object to exchange the contents with.
     */
    void swap(ByteArray &other) noexcept;

    /**
     * @brief Cast bytes in @c ByteArray started from the @c position into a value with the type
     * @c tIntegral.
     *
     * @note This function uses a @b system endianness for the conversion, thus you have to change
     * the endianness of the result if needed.
     *
     * @code
     * auto arr = ByteArray::fromHex("AA 00 01 Bc").value();
     * //                              0  1  2  3
     * auto value = arr.reinterpret<std::uint16_t>(1).value(); // for a little endian platform it
     *                                                    // takes bytes 0x00 and 0x01 and
     *                                                    // converts to the 0x0100 number
     * @endcode
     *
     * @tparam tIntegral Type to cast to, must be integral or @c std::byte.
     * @param position Position in the @c ByteArray from that casting is started.
     * @return Empty @c std::optional if <code>position + sizeof(tIntegral) > size()</code>,
     *         a constructed value otherwise.
     */
    template <typename tIntegral>
    [[nodiscard]] std::optional<tIntegral> reinterpret(std::size_t position) const noexcept;

    /**
     * @brief Insert a @c value to the @c ByteArray into the @c position specified.
     *
     * This function returns a reference to the @c ByteArray, thus it helps with a methods
     * chaining. If <code> index > size() </code>, then an exception is thrown.
     *
     * @note This function @b might trigger memory reallocation and active iterators invalidation.
     *
     * @note This function uses a @b system endianness for the conversion, thus you have to change
     * the endianness of the @c value if needed.
     *
     * @code
     * std::uint16_t one = 0x0011;
     * std::uint16_t two = 0x0022;
     * ByteArray arr;
     * // Let's consider this example as if there is a little endian machine.
     * arr.insertAsBytes(0, one) // content is {0x11, 0x00}
     *    .insertAsBytes(arr.size(), two); // content is {0x11, 0x00, 0x22, 0x00}
     *
     * @endcode
     *
     * @tparam tIntegral Type to save, must be integral or @c std::byte.
     * @param position  Position for the @c value insertion, legal values are in the range
     *                  <code> [0, size()] </code>.
     * @param value The value that is converted into the bytes and inserted into the @c ByteArrays.
     * @exception std::out_of_range if <code> index > size() </code>.
     */
    template <typename tIntegral>
    ByteArray &insertAsBytes(std::size_t position, const tIntegral &value) noexcept(false);

    /**
     * @brief Convert the data to a Base64 encoded string.
     *
     * @return Base64 encoded string.
     *
     * @sa @ref ByteArray::fromBase64
     *
     * @since 5.6
     */
    std::string toBase64() const noexcept;

private:
    Container m_data;
};

namespace bytearray_literals {

/**
 * @brief Convert a @e hexidecimal number into the @c ByteArray.
 *
 * This literal @b strictly requires a number in the @e hexidecimal format with @b even count of the
 * numbers.
 *
 * @code
 * using namespace literals;
 * ByteArray arr = 0xAB'00'CC_hex;
 * @endcode
 *
 * This code <code>1111_hex</code> causes compilation error, because number not in hexidecimal
 * format, and this snippet <code>0xA_hex</code> also leads to compilation error, because count of
 * numbers is not even.
 */
template <char... chars>
ByteArray operator"" _hex();

} // namespace bytearray_literals

template <typename tInputIt>
ByteArray::ByteArray(tInputIt begin, tInputIt end)
    : m_data(begin, end)
{
    using ValueType = typename std::iterator_traits<tInputIt>::value_type;
    static_assert(std::is_convertible_v<ValueType, std::byte>,
                  "Iterator has to hold value implicitly convertible into std::byte");
}


template <typename tIntegral>
std::optional<tIntegral> ByteArray::reinterpret(std::size_t position) const noexcept
{
    static_assert(std::is_integral_v<tIntegral> || std::is_same_v<tIntegral, std::byte>,
                  "Expected integral type or std::byte");
    // Regarding https://gcc.gnu.org/bugzilla/show_bug.cgi?id=80635
    if (position + sizeof(tIntegral) > size()) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
        return {};
#pragma GCC diagnostic pop
    }

    auto value = tIntegral{};
    std::memcpy(&value, &m_data[position], sizeof(value));
    return value;
}

template <typename tIntegral>
ByteArray &ByteArray::insertAsBytes(std::size_t position, const tIntegral &value) noexcept(false)
{
    static_assert(std::is_integral_v<tIntegral> || std::is_same_v<tIntegral, std::byte>,
                  "Expected integral type or std::byte");
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    m_data.insert(position, reinterpret_cast<const std::byte *>(&value), sizeof(value));
    return *this;
}

namespace details {

// Calculate count of hex-symbols in the literal. If the literal contains wrong symbols or not a
// hexidecimal, than 0 is returned.
constexpr std::size_t countHexNumbers(const char *str, std::size_t size)
{
    if (size < 2) {
        return 0;
    }

    if (str[0] != '0' || (str[1] != 'x' && str[1] != 'X')) {
        return 0;
    }
    std::size_t hexCount = 0;
    for (std::size_t i = 2; i < size; ++i) {
        char ch = str[i];
        if ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f')) {
            ++hexCount;
        } else if (ch != '\'') {
            return 0;
        }
    }
    return hexCount;
}

} // namespace details

template <char... chars>
ByteArray bytearray_literals::operator"" _hex()
{
    constexpr std::size_t size = sizeof...(chars);
    constexpr char str[size + 1] = {chars..., '\0'};
    constexpr std::size_t hexCount = details::countHexNumbers(str, size);
    static_assert(hexCount != 0, "The number is not hexadecimal");
    static_assert(hexCount % 2 == 0, "Wrong number of the symbols in the literal, should be even");
    return ByteArray::fromHex(std::string_view{str + 2, size - 2}, "'").value();
}

NF_END_NAMESPACE
