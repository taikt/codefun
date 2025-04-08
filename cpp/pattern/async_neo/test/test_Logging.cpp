/*
 * BMW Neo Framework
 *
 * Copyright (C) 2018-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 *     Szymon Wojtczak <szymon.wojtczak@globallogic.com>
 *     Nikolai Kutiavin <Nikolai.Kutiavin@bmw.de>
 */

#include <nf/Logging.h>
#include <nf/backend/MemoryLogger.h>
#include <nf/testing/Test.h>

using ::testing::_; // NOLINT
using ::testing::ContainsRegex;
using ::testing::InSequence;

NF_LOG_DEFINE_CONTEXT(nf_log_test1, CTX1, "");
NF_LOG_DEFINE_CONTEXT(nf_log_test2, CTX2, "");
NF_LOG_DEFINE_CONTEXT(nf_log_test3, CTX3, "");
NF_LOG_USE_CONTEXT(nf_log_test1);

namespace {

class LogTest : public ::testing::Test
{
public:
    void SetUp() override
    {
        m_log = std::make_shared<nf::backend::MemoryLogger>();
        nf::Logging::initialize({m_log});
    }

    void TearDown() override
    {
        nf::Logging::terminate();
    }

protected:
    std::shared_ptr<nf::backend::MemoryLogger> m_log;
};

class FallbackLogTest : public ::testing::Test
{
public:
    void TearDown() override
    {
        nf::Logging::terminate();
    }
};

class MockLogBackend : public nf::LogBackend
{
public:
    MOCK_METHOD(void, write, (const nf::LogContext &, nf::LogLevel level, const std::string &),
                (noexcept, override));
    MOCK_METHOD(void, write, (const nf::LogEntry &), (noexcept, override));
    MOCK_METHOD(void, suspendOutput, (), (noexcept, override));
    MOCK_METHOD(void, resumeOutput, (), (noexcept, override));

    MockLogBackend()
    {
        ON_CALL(*this, suspendOutput()).WillByDefault([this] {
            LogBackend::suspendOutput(); // coverage
        });
        ON_CALL(*this, resumeOutput()).WillByDefault([this] {
            LogBackend::resumeOutput(); // coverage
        });
    }
};

} // anonymous namespace

#define ASSERT_LOG(__idx, __ctxId, __msg)                                                          \
    ASSERT_EQ(__ctxId, entries[__idx].logContext.id);                                              \
    ASSERT_EQ(__msg, entries[__idx].message)

TEST_F(LogTest, log)
{
    const auto &context = NF_LOG_GET_CONTEXT(nf_log_test1);
    nf::log(context, nf::LogLevel::Debug, "Hello, {:d}", 1);
    nf::log(context, nf::LogLevel::Warn, "Hello, {}", 2);
    nf::log(context, nf::LogLevel::Info, FMT_STRING("Hey, {:d}"), 3);

    std::vector<nf::LogEntry> expected = {{context, nf::LogLevel::Debug, "Hello, 1", {}},
                                          {context, nf::LogLevel::Warn, "Hello, 2", {}},
                                          {context, nf::LogLevel::Info, "Hey, 3", {}}};
    EXPECT_EQ(expected, m_log->logEntries());
}

TEST_F(LogTest, log_logEntry)
{
    const auto &context = NF_LOG_GET_CONTEXT(nf_log_test1);
    nf::log(context, nf::LogLevel::Debug, "Hello, {:d}", 1);
    nf::log(context, nf::LogLevel::Warn, "Hello, {}", 2);
    nf::log(context, nf::LogLevel::Info, FMT_STRING("Hey, {:d}"), 3);

    std::vector<nf::LogEntry> expected = {{context, nf::LogLevel::Debug, "Hello, 1", {}},
                                          {context, nf::LogLevel::Warn, "Hello, 2", {}},
                                          {context, nf::LogLevel::Info, "Hey, 3", {}}};
    EXPECT_EQ(expected, m_log->logEntries());
}

TEST_F(LogTest, log_invalidFormat)
{
    const auto &context = NF_LOG_GET_CONTEXT(nf_log_test1);
    nf::log(context, nf::LogLevel::Debug, "Hello, {:s}", 1);

    std::vector<nf::LogEntry> expected = {
        {context, nf::LogLevel::Fatal, "Cannot log \"Hello, {:s}\": invalid type specifier", {}}};
    EXPECT_EQ(expected, m_log->logEntries());
}

TEST_F(LogTest, levelledLog_fileContext)
{
    const auto &context = NF_LOG_GET_CONTEXT(nf_log_test1);

    fatal("fatal {}", 0);
    error("error {} {}", 1, 2);
    warn("warn");
    info("info {}", 3);
    debug("{} 4", "debug");
    verbose("{} {}", "verbose", 5);

    std::vector<nf::LogEntry> expected = {{context, nf::LogLevel::Fatal, "fatal 0", {}},
                                          {context, nf::LogLevel::Error, "error 1 2", {}},
                                          {context, nf::LogLevel::Warn, "warn", {}},
                                          {context, nf::LogLevel::Info, "info 3", {}},
                                          {context, nf::LogLevel::Debug, "debug 4", {}},
                                          {context, nf::LogLevel::Verbose, "verbose 5", {}}};
    EXPECT_EQ(expected, m_log->logEntries());
}

TEST_F(LogTest, levelledLog_customContext)
{
    const auto &context = NF_LOG_GET_CONTEXT(nf_log_test2);

    nf::fatal(context, "fatal {}", 0);
    nf::error(context, "error {} {}", 1, 2);
    nf::warn(context, "warn");
    nf::info(context, "info {}", 3);
    nf::debug(context, "{} 4", "debug");
    nf::verbose(context, "{} {}", "verbose", 5);

    std::vector<nf::LogEntry> expected = {{context, nf::LogLevel::Fatal, "fatal 0", {}},
                                          {context, nf::LogLevel::Error, "error 1 2", {}},
                                          {context, nf::LogLevel::Warn, "warn", {}},
                                          {context, nf::LogLevel::Info, "info 3", {}},
                                          {context, nf::LogLevel::Debug, "debug 4", {}},
                                          {context, nf::LogLevel::Verbose, "verbose 5", {}}};
    EXPECT_EQ(expected, m_log->logEntries());
}

TEST_F(LogTest, levelledLog_moduleContext)
{
    const auto &context = NF_LOG_GET_CONTEXT(nf_core);

    nf::fatal("fatal {}", 0);
    nf::error("error {} {}", 1, 2);
    nf::warn("warn");
    nf::info("info {}", 3);
    nf::debug("{} 4", "debug");
    nf::verbose("{} {}", "verbose", 5);

    std::vector<nf::LogEntry> expected = {{context, nf::LogLevel::Fatal, "fatal 0", {}},
                                          {context, nf::LogLevel::Error, "error 1 2", {}},
                                          {context, nf::LogLevel::Warn, "warn", {}},
                                          {context, nf::LogLevel::Info, "info 3", {}},
                                          {context, nf::LogLevel::Debug, "debug 4", {}},
                                          {context, nf::LogLevel::Verbose, "verbose 5", {}}};
    EXPECT_EQ(expected, m_log->logEntries());
}

TEST_F(LogTest, useLocalContext)
{
    debug("One");
    {
        NF_LOG_USE_CONTEXT(nf_log_test2);
        debug("Two");
        {
            NF_LOG_USE_CONTEXT(nf_log_test3);
            debug("Three");
        }
        debug("Four");
    }
    debug("Five");

    const auto &entries = m_log->logEntries();
    ASSERT_EQ(5, entries.size());
    ASSERT_LOG(0, "CTX1", "One");
    ASSERT_LOG(1, "CTX2", "Two");
    ASSERT_LOG(2, "CTX3", "Three");
    ASSERT_LOG(3, "CTX2", "Four");
    ASSERT_LOG(4, "CTX1", "Five");
}

TEST_F(LogTest, duplicateContextIds)
{
    auto noCoreDumps = nf::testing::disableCoreDumps();
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    ASSERT_DEATH({ nf::LogContext ctx1("CTX1", "Re-defined CTX1"); }, "");
}

TEST_F(FallbackLogTest, useOfUninitalizedLogger)
{
    ASSERT_FALSE(nf::Logging::isInitialized());

    info("One");

    auto log = std::make_shared<nf::backend::MemoryLogger>();
    nf::Logging::initialize({log});

    ASSERT_EQ(log->logEntries().size(), 2);
    ASSERT_EQ(log->logEntries().at(0).logLevel, nf::LogLevel::Warn);
    ASSERT_EQ(log->logEntries().at(1).logLevel, nf::LogLevel::Info);
    ASSERT_THAT(log->logEntries().at(1).message, ContainsRegex(R"(\[CTX1\] One)"));
}

TEST_F(FallbackLogTest, useOfFilters)
{
    auto log = std::make_shared<nf::backend::MemoryLogger>();

    const auto *expectedContextId = ActiveLogContext.id;
    const auto *filteredContextId = NF_LOG_GET_CONTEXT(nf_log_test2).id;
    ASSERT_NE(expectedContextId, filteredContextId);

    using rule = nf::LogFilterRule;
    nf::Logging::initialize({log},
                            {rule::suppressContext(filteredContextId),
                             rule::suppressLevelsUpTo(nf::LogLevel::Info)});

    constexpr char expectedMsg[] = "ctx1 warning msg";

    info("ctx1 info msg");
    warn(expectedMsg);
    {
        NF_LOG_USE_CONTEXT(nf_log_test2);
        info("ctx2 info msg");
        warn("ctx2 warning msg");
    }

    const auto &entries = log->logEntries();
    ASSERT_EQ(1, entries.size());
    ASSERT_LOG(0, expectedContextId, expectedMsg);
}

TEST_F(FallbackLogTest, isInitialized)
{
    ASSERT_FALSE(nf::Logging::isInitialized());

    nf::Logging::initialize({});
    ASSERT_TRUE(nf::Logging::isInitialized());
    nf::Logging::terminate();
    ASSERT_FALSE(nf::Logging::isInitialized());

    nf::Logging::initialize({std::make_shared<nf::backend::MemoryLogger>()});
    ASSERT_TRUE(nf::Logging::isInitialized());
    nf::Logging::terminate();
    ASSERT_FALSE(nf::Logging::isInitialized());
}

TEST_F(FallbackLogTest, accessorMutator)
{
    auto memoryLogger0 = std::make_shared<nf::backend::MemoryLogger>();
    auto memoryLogger1 = std::make_shared<nf::backend::MemoryLogger>();
    auto mockLogger0 = std::make_shared<MockLogBackend>();
    auto mockLogger1 = std::make_shared<MockLogBackend>();

    nf::Logging::initialize({memoryLogger0});
    // Add
    nf::Logging::add(mockLogger0);
    nf::Logging::add(memoryLogger1);
    nf::Logging::add(mockLogger1);

    // Access
    ASSERT_EQ(memoryLogger0, nf::Logging::backend<nf::backend::MemoryLogger>().value());
    ASSERT_EQ(mockLogger1, nf::Logging::backend<MockLogBackend>(1).value());
    ASSERT_EQ(std::nullopt, nf::Logging::backend<nf::backend::MemoryLogger>(100)); // not found

    // Remove
    nf::Logging::remove<nf::backend::MemoryLogger>();
    ASSERT_EQ(std::nullopt, nf::Logging::backend<nf::backend::MemoryLogger>(1));
    nf::Logging::remove<MockLogBackend>(1);
    ASSERT_EQ(std::nullopt, nf::Logging::backend<MockLogBackend>(1));
    nf::Logging::remove<MockLogBackend>(1); // Does not crash when removed non existing

    // Terminate by removing and initialize by adding
    nf::Logging::remove<nf::backend::MemoryLogger>();
    nf::Logging::remove<MockLogBackend>();
    ASSERT_FALSE(nf::Logging::isInitialized());
    nf::Logging::add(mockLogger0);
    ASSERT_TRUE(nf::Logging::isInitialized());
}

TEST(LogFilterRuleTest, filterByContext)
{
    constexpr char filteredContext[] = "CTX";
    const auto ctxFilter = nf::LogFilterRule::suppressContext(filteredContext);

    EXPECT_TRUE(ctxFilter.appliesTo(filteredContext, nf::LogLevel::Fatal, "file.cpp"));
    EXPECT_TRUE(ctxFilter.appliesTo(filteredContext, nf::LogLevel::Info, ""));
    EXPECT_TRUE(ctxFilter.appliesTo(filteredContext, nf::LogLevel::Info, nullptr));

    constexpr char otherContext[] = "OTH";
    EXPECT_FALSE(ctxFilter.appliesTo(otherContext, nf::LogLevel::Verbose, "file.cpp"));

    constexpr char filteredFile[] = "filteredFile.cpp";
    const auto ctxLevelFileFilter =
        nf::LogFilterRule::suppressContext(filteredContext, nf::LogLevel::Info, filteredFile);

    EXPECT_TRUE(ctxLevelFileFilter.appliesTo(filteredContext, nf::LogLevel::Debug, filteredFile));
    EXPECT_TRUE(ctxLevelFileFilter.appliesTo(filteredContext, nf::LogLevel::Info, filteredFile));
    EXPECT_FALSE(ctxLevelFileFilter.appliesTo(filteredContext, nf::LogLevel::Warn, filteredFile));
    EXPECT_FALSE(ctxLevelFileFilter.appliesTo(otherContext, nf::LogLevel::Info, filteredFile));
    EXPECT_FALSE(ctxLevelFileFilter.appliesTo(filteredContext, nf::LogLevel::Info, "other.cpp"));
}

TEST(LogFilterRuleTest, filterByLevel)
{
    const auto infoLevelFilter = nf::LogFilterRule::suppressLevelsUpTo(nf::LogLevel::Info);

    EXPECT_TRUE(infoLevelFilter.appliesTo("CTX", nf::LogLevel::Debug, "file.cpp"));
    EXPECT_TRUE(infoLevelFilter.appliesTo("CTX", nf::LogLevel::Info, "file.cpp"));
    EXPECT_TRUE(infoLevelFilter.appliesTo("", nf::LogLevel::Info, "file.cpp"));
    EXPECT_TRUE(infoLevelFilter.appliesTo(nullptr, nf::LogLevel::Info, "file.cpp"));
    EXPECT_TRUE(infoLevelFilter.appliesTo("CTX", nf::LogLevel::Info, ""));
    EXPECT_TRUE(infoLevelFilter.appliesTo("CTX", nf::LogLevel::Info, nullptr));
    EXPECT_FALSE(infoLevelFilter.appliesTo("CTX", nf::LogLevel::Warn, "file.cpp"));

    const auto fatalLevelFilter = nf::LogFilterRule::suppressLevelsUpTo(nf::LogLevel::Fatal);
    EXPECT_TRUE(fatalLevelFilter.appliesTo("CTX", nf::LogLevel::Fatal, "file.cpp"));
    EXPECT_TRUE(fatalLevelFilter.appliesTo("CTX", nf::LogLevel::Error, "file.cpp"));
    EXPECT_TRUE(fatalLevelFilter.appliesTo("CTX", nf::LogLevel::Verbose, "file.cpp"));

    auto verboseLevelFilter = nf::LogFilterRule::suppressLevelsUpTo(nf::LogLevel::Verbose);
    EXPECT_FALSE(verboseLevelFilter.appliesTo("CTX", nf::LogLevel::Fatal, "file.cpp"));
    EXPECT_FALSE(verboseLevelFilter.appliesTo("CTX", nf::LogLevel::Debug, "file.cpp"));
    EXPECT_TRUE(verboseLevelFilter.appliesTo("CTX", nf::LogLevel::Verbose, "file.cpp"));
}

TEST(LogFilterRuleTest, filterByFile)
{
    constexpr char filteredFile[] = "filteredFile.cpp";
    const auto fileFilter = nf::LogFilterRule::suppressFile(filteredFile);

    EXPECT_TRUE(fileFilter.appliesTo("CTX", nf::LogLevel::Fatal, filteredFile));
    EXPECT_TRUE(fileFilter.appliesTo("OTH", nf::LogLevel::Fatal, filteredFile));
    EXPECT_TRUE(fileFilter.appliesTo("CTX", nf::LogLevel::Verbose, filteredFile));
    EXPECT_FALSE(fileFilter.appliesTo("CTX", nf::LogLevel::Fatal, "other.cpp"));
    EXPECT_FALSE(fileFilter.appliesTo("CTX", nf::LogLevel::Fatal, ""));
    EXPECT_FALSE(fileFilter.appliesTo("CTX", nf::LogLevel::Fatal, nullptr));

    const auto fileInfoLevelFilter =
        nf::LogFilterRule::suppressFile(filteredFile, nf::LogLevel::Info);

    EXPECT_TRUE(fileInfoLevelFilter.appliesTo("CTX", nf::LogLevel::Debug, filteredFile));
    EXPECT_TRUE(fileInfoLevelFilter.appliesTo("CTX", nf::LogLevel::Info, filteredFile));
    EXPECT_TRUE(fileInfoLevelFilter.appliesTo("OTH", nf::LogLevel::Info, filteredFile));
    EXPECT_FALSE(fileInfoLevelFilter.appliesTo("CTX", nf::LogLevel::Warn, filteredFile));
    EXPECT_FALSE(fileInfoLevelFilter.appliesTo("CTX", nf::LogLevel::Info, "other.cpp"));
    EXPECT_FALSE(fileInfoLevelFilter.appliesTo("CTX", nf::LogLevel::Info, ""));
    EXPECT_FALSE(fileInfoLevelFilter.appliesTo("CTX", nf::LogLevel::Info, nullptr));
}

TEST_F(FallbackLogTest, suspendedLogging)
{
    auto log = std::make_shared<MockLogBackend>();
    auto memoryLogger = std::make_shared<nf::backend::MemoryLogger>();
    nf::Logging::initialize({log, memoryLogger});
    const auto &context = NF_LOG_GET_CONTEXT(nf_log_test1);

    InSequence s;
    EXPECT_CALL(*log, suspendOutput());
    EXPECT_CALL(*log, write(_)).Times(1); // Send to the backend a request to write a log
    EXPECT_CALL(*log, resumeOutput());

    EXPECT_CALL(*log, suspendOutput());
    EXPECT_CALL(*log, write(_)).Times(1); // Send to the backend a request to write a log
    EXPECT_CALL(*log, resumeOutput());

    // First cycle during live time
    nf::Logging::suspendOutput();
    nf::log(context, nf::LogLevel::Debug, "Test Log");
    nf::Logging::resumeOutput();
    EXPECT_EQ(1,
              memoryLogger->logEntries().size()); // Check if no double writing to non-suspended log

    // Second cycle during live time
    nf::Logging::suspendOutput();
    nf::log(context, nf::LogLevel::Debug, "Test Log");
    nf::Logging::resumeOutput();
    EXPECT_EQ(2, memoryLogger->logEntries().size());
}

TEST_F(FallbackLogTest, resuspendReresume)
{

    auto log = std::make_shared<MockLogBackend>();
    nf::Logging::initialize({log});

    InSequence s;
    EXPECT_CALL(*log, suspendOutput()).Times(2);
    EXPECT_CALL(*log, resumeOutput()).Times(2);

    nf::Logging::suspendOutput();
    nf::Logging::suspendOutput();
    nf::Logging::resumeOutput();
    nf::Logging::resumeOutput();
}

TEST_F(FallbackLogTest, initializeSuspendStates)
{
    const auto &context = NF_LOG_GET_CONTEXT(nf_log_test1);
    const auto &defaultContext = NF_LOG_GET_CONTEXT(NF_LOG_CONTEXT);
    auto log = std::make_shared<nf::backend::MemoryLogger>();

    nf::log(context, nf::LogLevel::Debug, "Printed before suspend");
    // Suspend and resume on uninitialized logger
    nf::Logging::suspendOutput();
    nf::log(context, nf::LogLevel::Debug, "Printed before resume");
    nf::Logging::resumeOutput();
    nf::log(context, nf::LogLevel::Debug, "Printed after resume");

    // Initialize
    nf::Logging::initialize({log});
    nf::log(context, nf::LogLevel::Debug, "Printed to the logger");
    std::vector<nf::LogEntry> expectedAfterInit = {
        nf::LogEntry{defaultContext,
                     nf::LogLevel::Warn,
                     "These messages were logged before logging was initialized:",
                     {}},
        nf::LogEntry{defaultContext, nf::LogLevel::Debug, " ==> [CTX1] Printed before suspend", {}},
        nf::LogEntry{defaultContext,
                     nf::LogLevel::Warn,
                     " ==> [NFCO] Unable to suspend output of uninitialized logging",
                     {}},
        nf::LogEntry{defaultContext, nf::LogLevel::Debug, " ==> [CTX1] Printed before resume", {}},
        nf::LogEntry{defaultContext,
                     nf::LogLevel::Warn,
                     " ==> [NFCO] Unable to resume output of uninitialized logging",
                     {}},
        nf::LogEntry{defaultContext, nf::LogLevel::Debug, " ==> [CTX1] Printed after resume", {}},
        nf::LogEntry{context, nf::LogLevel::Debug, "Printed to the logger", {}}};
    EXPECT_EQ(expectedAfterInit, log->logEntries());

    // Suspend and resume on terminated logger
    nf::Logging::terminate();
    nf::log(context, nf::LogLevel::Debug, "After terminate");
    nf::Logging::suspendOutput();
    nf::log(context, nf::LogLevel::Debug, "Suspend after terminate");
    nf::Logging::resumeOutput();
    nf::log(context, nf::LogLevel::Debug, "Resume after terminate");

    log->clear();
    nf::Logging::initialize({log});

    std::vector<nf::LogEntry> expectedAfterTerminate =
        {nf::LogEntry{defaultContext,
                      nf::LogLevel::Warn,
                      "These messages were logged before logging was initialized:",
                      {}},
         nf::LogEntry{defaultContext, nf::LogLevel::Debug, " ==> [CTX1] After terminate", {}},
         nf::LogEntry{defaultContext,
                      nf::LogLevel::Warn,
                      " ==> [NFCO] Unable to suspend output of uninitialized logging",
                      {}},
         nf::LogEntry{defaultContext,
                      nf::LogLevel::Debug,
                      " ==> [CTX1] Suspend after terminate",
                      {}},
         nf::LogEntry{defaultContext,
                      nf::LogLevel::Warn,
                      " ==> [NFCO] Unable to resume output of uninitialized logging",
                      {}},
         nf::LogEntry{defaultContext,
                      nf::LogLevel::Debug,
                      " ==> [CTX1] Resume after terminate",
                      {}}};
    EXPECT_EQ(expectedAfterTerminate, log->logEntries());
}

TEST_F(FallbackLogTest, logBackends)
{
    {
        auto list = nf::Logging::logBackends();
        EXPECT_EQ(0, list.size());
    }

    nf::Logging::initialize(
        {std::make_shared<MockLogBackend>(), std::make_shared<nf::backend::MemoryLogger>()});
    auto list = nf::Logging::logBackends();
    EXPECT_EQ(list.size(), 2);
    EXPECT_TRUE(std::dynamic_pointer_cast<MockLogBackend>(list[0]));
    EXPECT_TRUE(std::dynamic_pointer_cast<nf::backend::MemoryLogger>(list[1]));
}
