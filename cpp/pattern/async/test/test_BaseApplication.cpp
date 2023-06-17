/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include <nf/ApplicationOptions.h>
#include <nf/BaseApplication.h>
#include <nf/TestBaseApplication.h>

constexpr char FooOptionName[] = "foo";
constexpr char FooOptionHelpText[] = "The foo parameter";

constexpr int FooDefaultValue = 2;

constexpr char ArglessOptionName[] = "arglessOption";
constexpr char ArglessOptionHelpText[] = "This is an option without argument value for testing";

constexpr char RunOptionName[] = "runOption";
constexpr char RunOptionHelpText[] = "Only if this option is specified, the application will start";

using namespace nf;
using namespace nf::testing;
using namespace ::testing;

class TestApplication final : public TestBaseApplication<BaseApplication>
{
public:
    TestApplication(std::initializer_list<const char *> args = {"--runOption"}) noexcept;

private:
    Result<bool, int> processCustomOptions(const ApplicationOptions &options) noexcept override;
    void fillOptions(ApplicationOptions &options) noexcept override;

public:
    int fooValue{0};
    bool arglessOptionValue{false};
};

TestApplication::TestApplication(std::initializer_list<const char *> args) noexcept
    : TestBaseApplication(args)
{
    allowMultipleInstances(); // Avoid issues with parallel test execution
}

Result<bool, int> TestApplication::processCustomOptions(const ApplicationOptions &options) noexcept
{
    int exitCode = 42;
    auto optFooValue = options.value<int>(FooOptionName, FooDefaultValue);
    bool run = options.isSpecified(RunOptionName);
    arglessOptionValue = options.isSpecified(ArglessOptionName);

    if (!optFooValue || !run) {
        return result::Err{exitCode};
    }

    fooValue = *optFooValue;
    return true;
}

void TestApplication::fillOptions(ApplicationOptions &options) noexcept
{
    options.add(RunOptionName, RunOptionHelpText);
    options.add<int>(FooOptionName, FooOptionHelpText);
    options.add(ArglessOptionName, ArglessOptionHelpText);
}

class BaseAppTest : public nf::testing::Test
{
public:
    void verifyHelpMessage(const std::string &helpMessage) const;
    void expectStdoutVersion(TestApplication &app, const char *regex);
};

void BaseAppTest::verifyHelpMessage(const std::string &message) const
{
    /* We only do a basic check if two shorter help texts are present. */
    EXPECT_NE(message.find(FooOptionHelpText), std::string::npos);
    EXPECT_NE(message.find(ArglessOptionHelpText), std::string::npos);
}

void BaseAppTest::expectStdoutVersion(TestApplication &app, const char *regex)
{
    ::testing::internal::CaptureStdout();
    int exitCode = app.run();

    EXPECT_THAT(::testing::internal::GetCapturedStdout(), ContainsRegex(regex));

    app.expectNoStartUpCalled();
    processEvents();
    app.expectNoStartUpCalled();

    EXPECT_EQ(EXIT_SUCCESS, exitCode);
}

TEST_F(BaseAppTest, startUp)
{
    TestApplication app;
    app.expectNoStartUpCalled();
    app.run([&] {
        processEvents();
        app.expectStartUpCalled();
    });
}

TEST_F(BaseAppTest, shutDown)
{
    TestApplication app;
    app.run([&](bool &isRunning) {
        processEvents();
        EXPECT_TRUE(isRunning);
        app.stop();
        processEvents();
        EXPECT_FALSE(isRunning);
    });
}

TEST_F(BaseAppTest, shutDownDefaultExitCode)
{
    TestApplication app;
    int exitCode = app.run([&](bool &isRunning) {
        processEvents();
        EXPECT_TRUE(isRunning);
        app.stop();
        processEvents();
        EXPECT_FALSE(isRunning);
    });
    EXPECT_EQ(EXIT_SUCCESS, exitCode);
}

TEST_F(BaseAppTest, shutDownUserDefinedExitCode)
{
    TestApplication app;
    int expectedCode = 42;
    int exitCode = app.run([&](bool &isRunning) {
        processEvents();
        EXPECT_TRUE(isRunning);
        app.stop(expectedCode);
        processEvents();
        EXPECT_FALSE(isRunning);
    });
    EXPECT_EQ(expectedCode, exitCode);
}

TEST_F(BaseAppTest, helpOption_ExitSucessful)
{
    TestApplication app({"--help", "--runOption"});
    ::testing::internal::CaptureStdout();
    int exitCode = app.run();
    verifyHelpMessage(::testing::internal::GetCapturedStdout());
    app.expectNoStartUpCalled();
    processEvents();
    app.expectNoStartUpCalled();

    EXPECT_EQ(EXIT_SUCCESS, exitCode);
}

TEST_F(BaseAppTest, helpShortOption_ExitSucessful)
{
    TestApplication app({"-h", "--runOption"});
    ::testing::internal::CaptureStdout();
    int exitCode = app.run();
    verifyHelpMessage(::testing::internal::GetCapturedStdout());
    app.expectNoStartUpCalled();
    processEvents();
    app.expectNoStartUpCalled();

    EXPECT_EQ(EXIT_SUCCESS, exitCode);
}

TEST_F(BaseAppTest, versionOption_notSpecified)
{
    TestApplication app({"--version", "--runOption"});
    expectStdoutVersion(app, ".+ \\(version not specified\\)");
}

TEST_F(BaseAppTest, versionOption)
{
    TestApplication app({"--version", "--runOption"});
    app.setBuildInfo("1.2");
    expectStdoutVersion(app, ".+ 1\\.2 \\(NF.+");
}

TEST_F(BaseAppTest, versionShortOption)
{
    TestApplication app({"-v", "--runOption"});
    app.setBuildInfo("3.4");
    expectStdoutVersion(app, ".+ 3\\.4 \\(NF.+");
}

TEST_F(BaseAppTest, defaultOptionValue)
{
    TestApplication app;
    app.run();
    app.expectNoStartUpCalled();
    processEvents();
    app.expectStartUpCalled();
    EXPECT_EQ(FooDefaultValue, app.fooValue);
    app.stop();
}

TEST_F(BaseAppTest, validoptionValue)
{
    TestApplication app({"", "--runOption", "--foo", "5"});
    app.run();
    app.expectNoStartUpCalled();
    processEvents();
    app.expectStartUpCalled();
    EXPECT_EQ(5, app.fooValue);
    app.stop();
}

TEST_F(BaseAppTest, invalidoptionValue_ExitFailure)
{
    TestApplication app({"", "--foo", "fxgg", "--runOption"});
    int exitCode = app.run();
    processEvents();
    app.expectNoStartUpCalled();

    EXPECT_EQ(EXIT_FAILURE, exitCode);
}

TEST_F(BaseAppTest, arglessOptionSpecified)
{
    TestApplication app({"", "--runOption", "--arglessOption"});
    app.run();
    app.expectNoStartUpCalled();
    processEvents();
    app.expectStartUpCalled();
    EXPECT_TRUE(app.arglessOptionValue);
    app.stop();
}

TEST_F(BaseAppTest, arglessOptionNotSpecified)
{
    TestApplication app;
    app.run();
    app.expectNoStartUpCalled();
    processEvents();
    app.expectStartUpCalled();
    EXPECT_FALSE(app.arglessOptionValue);
    app.stop();
}

TEST_F(BaseAppTest, optionNotStopApplication)
{
    TestApplication app;
    app.run();
    app.expectNoStartUpCalled();
    processEvents();
    app.expectStartUpCalled();
    app.stop();
}

TEST_F(BaseAppTest, optionStopApplication_ExitFailure)
{
    TestApplication app({""});
    int expectedCode = 42;
    int exitCode = app.run();
    app.expectNoStartUpCalled();
    processEvents();
    app.expectNoStartUpCalled();

    EXPECT_EQ(expectedCode, exitCode);
}

TEST_F(BaseAppTest, shutDownOnSigIntCalled)
{
    TestApplication app;
    app.run([&] {
        simulateUnixSignal(LinuxSignal::SigInt);
        EXPECT_EQ(1, app.cntShutDownCalled);
    });
}

TEST_F(BaseAppTest, shutDownOnSigTermCalled)
{
    TestApplication app;
    app.run([&] {
        simulateUnixSignal(LinuxSignal::SigTerm);
        EXPECT_EQ(1, app.cntShutDownCalled);
    });
}

TEST_F(BaseAppTest, logNoVersion)
{
    TestApplication app;
    app.run();
    EXPECT_THAT(logMessages(), Contains(ContainsRegex("Hello, my name is ut-Nf[5-9]Core!")));
}

TEST_F(BaseAppTest, logOnlyVersion)
{
    TestApplication app;
    app.setBuildInfo("4.2");
    app.run();
    EXPECT_THAT(logMessages(),
                Contains(
                    ContainsRegex("Hello, my name is .+ version 4.2 \\(NF [0-9](\\.[0-9]+)+\\)!")));
}

TEST_F(BaseAppTest, logVersionAndVariant)
{
    TestApplication app;
    app.setBuildInfo("4.2", "MGU18");
    app.run();
    EXPECT_THAT(logMessages(),
                Contains(ContainsRegex(
                    "Hello, my name is .+ version 4.2 \\(MGU18, NF [0-9](\\.[0-9]+)+\\)!")));
}

TEST_F(BaseAppTest, logAllBuildInfo)
{
    TestApplication app;
    app.setBuildInfo("4.2", "MGU18", "SDK X");
    app.run();
    EXPECT_THAT(logMessages(),
                Contains(
                    ContainsRegex(".+ version 4.2 \\(MGU18, NF [0-9](\\.[0-9]+)+, SDK X\\)!")));
}

TEST_F(BaseAppTest, logAllBuildInfoPeriodic)
{
    TestApplication app;
    app.setBuildInfo("4.2", "MGU18", "SDK X");
    app.enablePeriodicBuildInfo(std::chrono::seconds{60});
    app.run();
    processEvents(std::chrono::seconds{120});
    EXPECT_THAT(logMessages(),
                Contains(ContainsRegex(".+ version.*4.2 \\(MGU18, NF [0-9](\\.[0-9]+)+, SDK X\\)!"))
                    .Times(3));
}

TEST_F(BaseAppTest, terminate_intException)
{
    auto noCoreDumps = disableCoreDumps();
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wterminate"
#pragma GCC diagnostic ignored "-Wexceptions"
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    EXPECT_DEATH(
        {
            TestApplication app;
            // NOLINTNEXTLINE(bugprone-exception-escape)
            app.run([]() noexcept { throw 42; }); // NOLINT(hicpp-exception-baseclass)
        },
        "Terminating because of uncaught exception of type int");
#pragma GCC diagnostic pop
}

TEST_F(BaseAppTest, terminate_stdException)
{
    auto noCoreDumps = disableCoreDumps();
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wterminate"
#pragma GCC diagnostic ignored "-Wexceptions"
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    EXPECT_DEATH(
        {
            TestApplication app;
            // NOLINTNEXTLINE(bugprone-exception-escape)
            app.run([]() noexcept { throw std::runtime_error("ERRR"); });
        },
        "Terminating because of uncaught exception of type std::runtime_error: ERRR");
#pragma GCC diagnostic pop
}

TEST_F(BaseAppTest, terminate_explicit)
{
    auto noCoreDumps = disableCoreDumps();
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    EXPECT_DEATH(
        {
            TestApplication app;
            app.run([] { std::terminate(); });
        },
        "Terminating because std::terminate was explicitly called");
}

TEST_F(BaseAppTest, terminate_intException_reaches_BaseApplication)
{
    auto noCoreDumps = disableCoreDumps();
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    EXPECT_THROW(
        {
            TestApplication app;
            app.run([]() { throw 42; }); // NOLINT(hicpp-exception-baseclass)
        },
        int);
}

TEST_F(BaseAppTest, terminate_stdException_reaches_BaseApplication)
{
    auto noCoreDumps = disableCoreDumps();
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    EXPECT_THROW(
        {
            TestApplication app;
            app.run([]() { throw std::runtime_error("ERRR"); });
        },
        std::runtime_error);
}

#ifdef NF_TEST_ROOT_MODE
TEST_F(BaseAppTest, forbidRunAsRoot)
{
    auto noCoreDumps = disableCoreDumps();
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-goto)
    EXPECT_DEATH(
        {
            TestApplication app;
            app.forceForbidRunAsRoot();
            app.run();
        },
        "This application cannot be run as root");
}
#endif
