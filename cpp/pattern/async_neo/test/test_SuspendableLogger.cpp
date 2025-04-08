/*
 * BMW Neo Framework
 *
 * Copyright (C) 2023 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Nikolai Kutiavin <Nikolai.Kutiavin@bmw.de>
 */

#include <nf/SuspendableLogger.h>
#include <nf/testing/Test.h>

NF_LOG_DEFINE_CONTEXT(nf_log_Suspend, SUSL, "");

class MockLogBackend : public nf::SuspendableLogger
{
public:
    MOCK_METHOD(void, output, (const nf::LogEntry &), (noexcept, override));
    MOCK_METHOD(void, aboutToSuspend, (), (noexcept, override));
    MOCK_METHOD(void, aboutToResume, (), (noexcept, override));
};

TEST(SuspendableLoggerTest, suspend_resume_expectedCorrectValueOf_isSuspended)
{
    MockLogBackend logger;
    EXPECT_FALSE(logger.isSuspended());
    logger.suspendOutput();
    EXPECT_TRUE(logger.isSuspended());
    logger.resumeOutput();
    EXPECT_FALSE(logger.isSuspended());
}

TEST(SuspendableLoggerTest, logMessagesWithoutSuspendExpectAllWritten)
{
    MockLogBackend logger;
    const auto &context = NF_LOG_GET_CONTEXT(nf_log_Suspend);

    ::testing::InSequence s;
    EXPECT_CALL(logger, output(nf::LogEntry{context, nf::LogLevel::Debug, "not suspended 1", {}}));
    EXPECT_CALL(logger, output(nf::LogEntry{context, nf::LogLevel::Debug, "not suspended 2", {}}));

    logger.write({context, nf::LogLevel::Debug, "not suspended 1", {}});
    logger.write({context, nf::LogLevel::Debug, "not suspended 2", {}});
}

TEST(SuspendableLoggerTest, suspendThenLog_expectNoLogMessagePrinted)
{
    MockLogBackend logger;
    const auto &context = NF_LOG_GET_CONTEXT(nf_log_Suspend);

    ::testing::InSequence s;
    EXPECT_CALL(logger, aboutToSuspend).Times(1);
    EXPECT_CALL(logger, output).Times(0);

    logger.suspendOutput();
    logger.write({context, nf::LogLevel::Debug, "not suspended 1", {}});
}

TEST(SuspendableLoggerTest, suspendThenLogThenResume_expectedLogMessage)
{
    MockLogBackend logger;
    const auto &context = NF_LOG_GET_CONTEXT(nf_log_Suspend);

    ::testing::InSequence s;
    EXPECT_CALL(logger, aboutToSuspend).Times(1);
    EXPECT_CALL(logger, aboutToResume).Times(1);
    EXPECT_CALL(logger, output(nf::LogEntry{context, nf::LogLevel::Debug, "not suspended 1", {}}));

    logger.suspendOutput();
    logger.write({context, nf::LogLevel::Debug, "not suspended 1", {}});
    logger.resumeOutput();
}