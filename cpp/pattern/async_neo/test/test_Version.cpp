/*
 * BMW Neo Framework
 *
 * Copyright (C) 2020 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Wawrzyniec Kaczmarowski <wawrzyniec.kaczmarowski@partner.bmw.de>
 */

#include <nf/Version.h>
#include <nf/testing/Test.h>
#include <nf/testing/TestIoStream.h>

#include <sstream>
#include <string>

using namespace nf;
using namespace nf::testing;

TEST(VersionTest, VersionCreation)
{
    Version version(1, 2, 3);

    EXPECT_EQ(1, version.majorVersion());
    EXPECT_EQ(2, version.minorVersion());
    EXPECT_EQ(3, version.patch());
}

TEST(VersionTest, VersionCreationPreRelease)
{
    std::string preRelease("1.ew.456");
    std::string emptyString;

    Version version(1, 2, 3, preRelease);

    EXPECT_EQ(1, version.majorVersion());
    EXPECT_EQ(2, version.minorVersion());
    EXPECT_EQ(3, version.patch());
    EXPECT_EQ(preRelease, version.preRelease());
    EXPECT_EQ(emptyString, version.metaData());
}

TEST(VersionTest, VersionCreationPreReleaseAndMetaData)
{
    std::string preRelease("1.ew.456");
    std::string metaData("meta.exp.45424e");
    std::string emptyString;

    Version version(1, 2, 3, preRelease, metaData);

    EXPECT_EQ(1, version.majorVersion());
    EXPECT_EQ(2, version.minorVersion());
    EXPECT_EQ(3, version.patch());
    EXPECT_EQ(preRelease, version.preRelease());
    EXPECT_EQ(metaData, version.metaData());
}

TEST(VersionTest, VersionPrint)
{
    std::string preRelease("1.ew.456");
    std::string metaData("meta.exp.45424e");

    Version version(1, 2, 3, preRelease, metaData);
    Version version2(1, 2, 3);

    EXPECT_EQ("v1.2.3-1.ew.456+meta.exp.45424e", streamToString(version));
    EXPECT_EQ("v1.2.3", streamToString(version2));
}

TEST(VersionTest, VersionComparasion)
{
    ASSERT_LT(Version(2, 3, 4), Version(3, 3, 4));
    ASSERT_LT(Version(2, 3, 4), Version(2, 4, 4));
    ASSERT_LT(Version(2, 3, 4), Version(2, 3, 5));

    ASSERT_LT(Version(1, 3, 4), Version(2, 3, 4));
    ASSERT_LT(Version(2, 2, 4), Version(2, 3, 4));
    ASSERT_LT(Version(2, 3, 3), Version(2, 3, 4));

    ASSERT_FALSE(Version(2, 3, 4) < Version(2, 3, 4));

    ASSERT_LT(Version(2, 3, 4, "1.7.ax"), Version(2, 3, 4, "1.7.ay"));
    ASSERT_FALSE(Version(2, 3, 4, "1.7.ay") < Version(2, 3, 4, "1.7.ay"));
    ASSERT_LT(Version(2, 3, 4), Version(2, 3, 4, "1.7.ax"));
}
