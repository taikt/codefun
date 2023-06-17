/*
 * BMW Neo Framework
 *
 * Copyright (C) 2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Nikolay Kutyavin <nikolay.kutyavin@bmw.de>
 */

#include <nf/ByteArray.h>


using namespace nf;
using namespace nf::bytearray_literals;



#ifdef NF_BFT_0
struct StructWithNumber
{
    std::uint8_t number;
};
void ReinterpretToStruct()
{
    ByteArray arr = ByteArray::fromHex("00 11 22 33").value();
    [[maybe_unused]] auto val = arr.reinterpret<StructWithNumber>(0);
}
#endif

#ifdef NF_BFT_1
enum class SimpleEnum
{
    One,
    Two
};
void ReinterpretToEnum()
{
    ByteArray arr = ByteArray::fromHex("00 11 22 33").value();
    [[maybe_unused]] auto val = arr.reinterpret<SimpleEnum>(0);
}
#endif

#ifdef NF_BFT_2
struct StructWithNumber
{
    std::uint8_t number;
};
void InsertAsBytesStruct()
{
    StructWithNumber value = {10};
    ByteArray arr;
    arr.insertAsBytes(0, value);
}
#endif

#ifdef NF_BFT_3
enum class SimpleEnum
{
    One,
    Two
};
void InsertAsBytesEnum()
{
    ByteArray arr;
    arr.insertAsBytes(0, SimpleEnum::One);
}
#endif

#ifdef NF_BFT_4
void UseLiteralForDouble()
{
    [[maybe_unused]] auto arr = 0.1_hex;
}
#endif

#ifdef NF_BFT_5
void UseLiteralForString()
{
    [[maybe_unused]] auto arr = "00 11"_hex;
}
#endif

#ifdef NF_BFT_6
void UseLiteralForProperDecimalNumber()
{
    [[maybe_unused]] auto arr = 912_hex;
}
#endif

#ifdef NF_BFT_7
void UseLiteralForProperOctalNumber()
{
    [[maybe_unused]] auto arr = 07123_hex;
}
#endif

#ifdef NF_BFT_8
void UseLiteralForProperBinaryNumber()
{
    [[maybe_unused]] auto arr = 0b01011_hex;
}
#endif

#ifdef NF_BFT_9
void UseLiteralForFloatingPointNumber()
{
    [[maybe_unused]] auto arr = 4e2_hex;
}
#endif

#ifdef NF_BFT_10
void UseLiteralForFloatingPointNumberInHex()
{
    [[maybe_unused]] auto arr = 0x13p51_hex;
}
#endif

#ifdef NF_BFT_11
void UseLiteralForHexadecimalNumberWithOddCount()
{
    [[maybe_unused]] auto arr = 0x00'BB'F_hex;
}
#endif