/**
 *  Copyright (c) 2019 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/**
 * @file       ThreadDispatcher_UnitTest.cpp
 *
 * @brief      This class performs tests for ThreadDispatcher class
 */

#include <chrono>
#include <gtest/gtest.h>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <future>

#include "common/Logger.hpp"
#include "common/TaskDispatcher.hpp"

#include "TestConfigs.hpp"
#include "TestSetup.hpp"
#include "TestUtils.hpp"

class TaskDispatcherTest : public TestSetup {
 public:
    void SetUp() override {
        taskDispatcher_ = std::unique_ptr<telux::common::TaskDispatcher>(
            new telux::common::TaskDispatcher(threadSize_));
    }

    void TearDown() override {
    }

    std::unique_ptr<telux::common::TaskDispatcher> taskDispatcher_;
    const int threadSize_ = 2;
    bool isFuncCalled_ = false;
};

TEST_F(TaskDispatcherTest, validateStdFunction) {
    LOG(DEBUG, "validateStdFunction -- START ");
    std::function<void()> func = [this]() {
        LOG(DEBUG, "inside std::function ");
        isFuncCalled_ = true;
    };
    taskDispatcher_->submitTask(func);
    TestUtils::sleepInSeconds(1);
    ASSERT_TRUE(isFuncCalled_);
    LOG(DEBUG, "validateStdFunction -- END ");
}

void testFunction(int a) {
    LOG(DEBUG, __FUNCTION__, " executed with value: ", a);
}

TEST_F(TaskDispatcherTest, validateBindFunction) {
    LOG(DEBUG, "validateBindFunction -- START ");
    taskDispatcher_->submitTask(std::bind(&testFunction, 1));
    LOG(DEBUG, "validateBindFunction -- END ");
}

TEST_F(TaskDispatcherTest, validateMultipleBindFunction) {
    LOG(DEBUG, "validateMultipleBindFunction -- START ");
    taskDispatcher_->submitTask(std::bind(&testFunction, 111));
    taskDispatcher_->submitTask(std::bind(&testFunction, 222));
    taskDispatcher_->submitTask(std::bind(&testFunction, 333));
    for (int i = 0; i < 10; i++) {
        taskDispatcher_->submitTask(std::bind(&testFunction, i));
    }
    TestUtils::sleepInSeconds(5);
    LOG(DEBUG, "validateMultipleBindFunction -- END ");
}

using resultCb = std::function<void(int result)>;
void resultCallback(int result) {
    LOG(DEBUG, "Result Callback Output: ", result);
}

void multiply(int a, int b, resultCb cb) {
    if (cb) {
        cb(a * b);
    }
}

TEST_F(TaskDispatcherTest, bindFunctionWithCallback) {
    LOG(DEBUG, "bindFunctionWithCallback -- START ");
    taskDispatcher_->submitTask(std::bind(&multiply, 5, 5, &resultCallback));
    TestUtils::sleepInSeconds(1);
    LOG(DEBUG, "bindFunctionWithCallback -- END ");
}

int add(int a, int b) {
    // LOG(DEBUG,__FUNCTION__);
    return a + b;
}
TEST_F(TaskDispatcherTest, bindFunctionWithReturnValue) {
    LOG(DEBUG, "bindFunctionWithReturnValue -- START ");
    auto f = taskDispatcher_->submitTask(std::bind(&add, 5, 5));
    LOG(DEBUG, "bindFunctionWithReturnValue value = ", f.get());
    LOG(DEBUG, "bindFunctionWithReturnValue -- END ");
}

TEST_F(TaskDispatcherTest, occupyAllThreads) {
    for (int i = 0; i < threadSize_; i++) {
        auto f = taskDispatcher_->submitTask([]() { TestUtils::sleepInSeconds(2); });
    }
    LOG(DEBUG, "After submission");
    TestUtils::sleepInSeconds(5);
    LOG(DEBUG, "end");
}

TEST_F(TaskDispatcherTest, occupyAllThreadsWithTenTasks) {
    for (int i = 0; i < threadSize_ * 5; i++) {
        auto f = taskDispatcher_->submitTask([]() { TestUtils::sleepInSeconds(2); });
    }
    LOG(DEBUG, "After submission");
    TestUtils::sleepInSeconds(5);
    LOG(DEBUG, "end");
}
