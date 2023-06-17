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

#include <nf/ContainerAlgorithms.h>
#include <nf/testing/Test.h>

#include <deque>
#include <list>
#include <queue>
#include <string>
#include <unordered_set>

using namespace nf;
using namespace nf::testing;

TEST(ContainerAlgorithmsTest, containsKey_int2string)
{
    std::map<int, std::string> map;
    map[2] = "valueTwo";
    map[4] = "valueFour";
    EXPECT_TRUE(containsKey(map, 2));
    EXPECT_FALSE(containsKey(map, 3));
}

TEST(ContainerAlgorithmsTest, containsKey_int2int)
{
    /* map with mapped_type = value_type */
    std::map<int, int> map;
    map[1] = 2;
    map[2] = 3;
    map[4] = 5;
    EXPECT_TRUE(containsKey(map, 2));
    EXPECT_FALSE(containsKey(map, 3));
}

TEST(ContainerAlgorithmsTest, containsKey_transparent)
{
    /* transparent comparator */
    std::map<std::string, int, std::less<>> map;
    map["alpha"] = 1;
    map["beta"] = 2;
    EXPECT_TRUE(containsKey(map, "alpha"));
    EXPECT_TRUE(containsKey(map, std::string_view("alpha")));
    EXPECT_FALSE(containsKey(map, "gamma"));
    EXPECT_FALSE(containsKey(map, std::string_view("gamma")));
}

/// Verify containsValue() function.
TEST(ContainerAlgorithmsTest, containsValue)
{
    /* map */
    std::map<int, std::string> myMap;
    myMap[2] = "valueTwo";
    myMap[4] = "valueFour";
    EXPECT_TRUE(containsValue(myMap, std::string("valueTwo")));
    EXPECT_FALSE(containsValue(myMap, std::string("valueThree")));

    /* map with mapped_type = value_type */
    std::map<int, int> sameTypesMap;
    sameTypesMap[1] = 2;
    sameTypesMap[2] = 3;
    sameTypesMap[4] = 5;
    EXPECT_TRUE(containsValue(sameTypesMap, 3));
    EXPECT_FALSE(containsValue(sameTypesMap, 4));
}

/// Verify contains() function.
TEST(ContainerAlgorithmsTest, contains)
{
    /* map */
    std::map<int, int> myMap;
    myMap[1] = 2;
    EXPECT_TRUE(contains(myMap, std::pair<const int, int>(1, 2)));
    EXPECT_FALSE(contains(myMap, std::pair<const int, int>(1, 3)));

    /* set */
    std::set<double> mySet;
    mySet.insert(1);
    mySet.insert(3);
    mySet.insert(5);
    EXPECT_TRUE(contains(mySet, 3.0));
    EXPECT_FALSE(contains(mySet, 2.0));

    /* vector */
    std::vector<std::string> myVec;
    myVec.emplace_back("hello");
    myVec.emplace_back("world");
    myVec.emplace_back("lorem");
    myVec.emplace_back("ipsum");
    EXPECT_TRUE(contains(myVec, std::string("ipsum")));
    EXPECT_FALSE(contains(myVec, std::string("welt")));
}

/// Verify containsIf() function.
TEST(ContainerAlgorithmsTest, containsIf)
{
    /* User defined type */
    struct Dummy
    {
        int number{0};
        std::string message;
    };

    std::vector<Dummy> vec = {{1, "hello"}, {2, "world"}};

    auto predicate = [](const Dummy &dummy) { return dummy.message == "hello world"; };

    EXPECT_FALSE(containsIf(vec, predicate));
    EXPECT_TRUE(containsIf(vec, [](const Dummy &dummy) { return dummy.number == 2; }));
}

/// Verify find function.
TEST(ContainerAlgorithmsTest, find)
{
    /* A set of integers. */
    const std::set<int> numSet = {10, 20, 30};

    auto element = find(numSet, 30);

    EXPECT_EQ(30, *element);
    EXPECT_EQ(std::nullopt, find(numSet, 50));

    /* User defined type */
    struct Dummy
    {
        int number{0};
        std::string message;

        bool operator==(const Dummy &other) const
        {
            return number == other.number && message == other.message;
        }
    };

    /* A vector with a user defined type. */
    const Dummy dummy{3, "foobar"};
    std::vector<Dummy> dummies = {{1, "bla"}, {2, "blabla"}, dummy, {4, "barfoo"}};

    auto mutableElement = find(dummies, dummy);
    mutableElement->get().number = 10;

    EXPECT_EQ(10, dummies[2].number);
}

/// Verify findByKey function for small datatypes.
TEST(ContainerAlgorithmsTest, findByKey_builtInTypes)
{
    // clang-format off
    enum class SomeEnum { Val1, Val2, Val3 };
    // clang-format on

    /* map */
    const std::map<SomeEnum, int> myMap = {std::pair<SomeEnum, int>{SomeEnum::Val2, 2},
                                           std::pair<SomeEnum, int>{SomeEnum::Val3, 3}};

    EXPECT_EQ(2, *findByKey(myMap, SomeEnum::Val2));
    EXPECT_EQ(std::nullopt, findByKey(myMap, SomeEnum::Val1));

    /* ensure this works */
    EXPECT_EQ(2, findByKey(myMap, SomeEnum::Val2).value_or(5));
    EXPECT_EQ(5, findByKey(myMap, SomeEnum::Val1).value_or(5));
}

/// Verify findByKey function for complex datatypes.
TEST(ContainerAlgorithmsTest, findByKey_UserDefinedTypes)
{
    // clang-format off
    /* Type with sizeof(SomeType) > sizeof(void*). */
    struct SomeType { int a; void *b; };
    // clang-format on

    /* map */
    std::map<int, SomeType> myMap;
    const SomeType one = {1, nullptr};
    const SomeType two = {2, nullptr};
    myMap[2] = one;
    myMap[4] = two;
    EXPECT_EQ(one.a, findByKey(myMap, 2)->get().a);
    EXPECT_EQ(std::nullopt, findByKey(myMap, 3));
}

/// Verify usage of findByKey function to change container elements.
TEST(ContainerAlgorithmsTest, findByKey_)
{
    // clang-format off
    enum class SomeEnum { Val1, Val2, Val3 };
    // clang-format on

    /* map with int */
    std::map<SomeEnum, int> intMap;
    intMap[SomeEnum::Val2] = 2;
    intMap[SomeEnum::Val3] = 3;

    EXPECT_EQ(std::nullopt, findByKey(intMap, SomeEnum::Val1));
    auto optRef2 = findByKey(intMap, SomeEnum::Val2);
    EXPECT_EQ(2, *optRef2);

    optRef2->get() = 5;
    EXPECT_EQ(5, intMap[SomeEnum::Val2]);

    /* map with string */
    std::map<int, std::string> stringMap;
    stringMap[2] = "valueTwo";
    stringMap[4] = "valueFour";

    EXPECT_EQ(std::nullopt, findByKey(stringMap, 3));
    auto optRef4 = findByKey(stringMap, 4);
    EXPECT_EQ("valueFour", optRef4->get());

    optRef4->get() = "someNewString";
    EXPECT_EQ("someNewString", stringMap[4]);
}

/// Verify findIf function.
TEST(ContainerAlgorithmsTest, findIf)
{
    /* A set of integers. Note that it would be more suitable to use simply find here.
       Function findIf is use only to demonstrate that it can be used with sets as well. */
    const std::set<int> numSet = {10, 20, 30};

    const auto element = findIf(numSet, [](const int &elem) { return elem == 20; });
    EXPECT_EQ(20, *element);
    EXPECT_EQ(std::nullopt, findIf(numSet, [](const int &elem) { return elem == 100; }));

    /* User defined type */
    struct Dummy
    {
        int number{0};
        std::string message;
    };

    /* A vector with a user defined type. */
    std::vector<Dummy> dummies = {{3, "foobar"}, {4, "barfoo"}};

    const auto predicate = [](const Dummy &elem) { return elem.message == "barfoo"; };
    auto mutableElement = findIf(dummies, predicate);

    mutableElement->get().number = 10;

    auto modifiedElement = findIf(dummies, std::move(predicate));

    EXPECT_EQ(10, modifiedElement->get().number);
}

/// Transform vector to vector.
TEST(ContainerAlgorithmsTest, transformToVector_vector)
{
    std::vector<int> container = {1, 2, 3};

    std::vector<int> expectedIntVec = {2, 3, 4};
    auto intVec = transformToVector(container, [](int val) { return val + 1; });
    EXPECT_EQ(expectedIntVec, intVec);

    std::vector<std::string> expectedStringVec = {"1", "2", "3"};
    auto stringVec = transformToVector(container, [](int val) { return std::to_string(val); });
    EXPECT_EQ(expectedStringVec, stringVec);
}

/// Transform map to vector.
TEST(ContainerAlgorithmsTest, transformToVector_map)
{
    std::map<int, std::string> container = {{1, "1"}, {2, "2"}, {3, "3"}};

    std::vector<int> expectedKeyVec = {1, 2, 3};
    auto keyVec = transformToVector(container, [](const auto &entry) { return entry.first; });
    EXPECT_EQ(expectedKeyVec, keyVec);

    std::vector<std::string> expectedValueVec = {"1", "2", "3"};
    auto valueVec = transformToVector(container, [](const auto &entry) { return entry.second; });
    EXPECT_EQ(expectedValueVec, valueVec);
}

TEST(ContainerAlgorithmsTest, insertBackIf)
{
    std::vector<int> input = {-2, 3, 0, -7, -2, 1, 2};
    std::vector<int> output;

    insertBackIf(input, output, [](int i) { return i > 0; });

    std::vector<int> expectation = {3, 1, 2};
    EXPECT_EQ(expectation, output);
}

TEST(ContainerAlgorithmsTest, insertIf)
{
    std::vector<int> input = {-2, 3, 0, -7, -2, 1, 2};
    std::unordered_set<int> output;

    insertIf(input, output, [](int i) { return i < 0; });

    std::unordered_set<int> expectation = {-7, -2};
    EXPECT_EQ(expectation, output);
}

TEST(ContainerAlgorithmsTest, erase)
{
    /* Erase even elements from a vector of integers. */
    std::vector<int> vect{-3, -2, -1, 0, 1, 2, 3};
    eraseIf(vect, [](int i) { return i % 2 == 0; });

    const std::vector<int> expectedVect{-3, -1, 1, 3};
    EXPECT_EQ(expectedVect, vect);

    /* Erase positive elements from a deque of doubles. */
    std::deque<double> deq{-3.0, -2.0, -1.0, 0.0, 1.0, 2.0, 3.0};
    eraseIf(deq, [](double d) { return d > 0.0; });

    const std::deque<double> expectedDeq{-3.0, -2.0, -1.0, 0.0};
    EXPECT_EQ(expectedDeq, deq);

    /* Erase empty strings from a list. */
    std::list<std::string> strings{"foo", "", "bar", "", "foobar"};
    eraseIf(strings, [](const std::string &str) { return str.empty(); });

    const std::list<std::string> expectedStrings{"foo", "bar", "foobar"};
    EXPECT_EQ(expectedStrings, strings);
}

TEST(ContainerAlgorithmsTest, pop_queue)
{
    std::queue<int> queue{{1, 2, 3, 4, 5}};
    const std::queue<int> expectedQueue{{3, 4, 5}};

    auto first = pop(queue);
    auto second = pop(queue);

    EXPECT_EQ(1, first);
    EXPECT_EQ(2, second);
    EXPECT_EQ(expectedQueue, queue);
}

TEST(ContainerAlgorithmsTest, pop_priorityQueue)
{
    std::priority_queue<int> queue;
    queue.push(1);
    queue.push(5);
    queue.push(3);
    queue.push(4);
    queue.push(2);

    auto first = pop(queue);
    auto second = pop(queue);

    EXPECT_EQ(5, first);
    EXPECT_EQ(4, second);
    EXPECT_EQ(3, queue.size());
}
