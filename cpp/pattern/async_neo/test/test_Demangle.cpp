/*
 * BMW Neo Framework
 *
 * Copyright (C) 2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Florian Franzmann <Florian.Franzmann@ul.com>
 *     Manuel Nickschas <manuel.nickschas@bmw.de>
 */

#include <nf/Demangle.h>
#include <nf/Signal.h>
#include <nf/testing/Test.h>

#include <iostream>
#include <utility>

class Foo
{
};

namespace nf::test {
class Bar
{
};
struct Foo42
{
};
} // namespace nf::test

TEST(DemangleTest, demanglesClassName)
{
    auto name = nf::demangle<Foo>();

    ASSERT_EQ(name, "Foo");
}

TEST(DemangleTest, classNameWithNamespace)
{
    auto name = nf::demangle<nf::test::Bar>();

    ASSERT_EQ(name, "nf::test::Bar");
}

TEST(DemangleTest, removesNamespace)
{
    auto name = nf::demangleWithoutNamespace<nf::test::Bar>();

    ASSERT_EQ(name, "Bar");
}

TEST(DemangleTest, removesNamespace_leavesTemplateArgumentIntact)
{
    auto name = nf::demangleWithoutNamespace<
        nf::Signal<std::pair<nf::test::Foo42, char *>, std::byte, nf::test::Bar>>();

    ASSERT_EQ(name, "Signal<pair<Foo42, char*>, byte, Bar>");
}
