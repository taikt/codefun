/*
 * BMW Neo Framework
 *
 * Copyright (C) 2019-2022 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * Contributors:
 *     Antons Jeļkins <antons.jelkins@bmw.de>
 */

#include <nf/BaseApplication.h>
#include <nf/testing/Test.h>

NF_BEGIN_NAMESPACE

template <typename tBaseApplication>
class TestBaseApplication : public tBaseApplication
{
public:
    static const char **makeArgs(std::initializer_list<const char *> args) noexcept;

public:
    TestBaseApplication(std::initializer_list<const char *> args) noexcept;

private:
    Future<void> startUp() noexcept override;
    Future<void> shutDown() noexcept override;
    void initializeLogging() noexcept override;
    void initializeFramework() noexcept override;
    void terminateFramework() noexcept override;

public:
    int run();
    int run(nf::testing::OnRunAction action);
    int run(const std::function<void()> &action);

    void forceForbidRunAsRoot() noexcept;
    void expectStartUpCalled() const;
    void expectNoStartUpCalled() const;
    void stop(int exitCode = EXIT_SUCCESS);

    int cntStartUpCalled{0};
    int cntShutDownCalled{0};

private:
    bool m_isNoRootForced{false};
};

template <typename tBaseApplication>
const char **
TestBaseApplication<tBaseApplication>::makeArgs(std::initializer_list<const char *> args) noexcept
{
    static std::vector<const char *> s_args;
    s_args.clear();
    s_args.push_back("test_app_name");
    s_args.insert(std::end(s_args), args);
    return s_args.data();
}

template <typename tBaseApplication>
TestBaseApplication<tBaseApplication>::TestBaseApplication(
    std::initializer_list<const char *> args) noexcept
    : tBaseApplication(static_cast<int>(args.size()) + 1, makeArgs(args))
{
}

template <typename tBaseApplication>
Future<void> TestBaseApplication<tBaseApplication>::startUp() noexcept
{
    cntStartUpCalled++;
    return {};
}

template <typename tBaseApplication>
Future<void> TestBaseApplication<tBaseApplication>::shutDown() noexcept
{
    cntShutDownCalled++;
    return {};
}

template <typename tBaseApplication>
void TestBaseApplication<tBaseApplication>::initializeLogging() noexcept
{
    /* Initialized in the test suite. */
}

template <typename tBaseApplication>
void TestBaseApplication<tBaseApplication>::initializeFramework() noexcept
{
    /* Initialized in the test suite. */
}

template <typename tBaseApplication>
void TestBaseApplication<tBaseApplication>::terminateFramework() noexcept
{
    /* Terminated in the test suite. */
}

template <typename tBaseApplication>
int TestBaseApplication<tBaseApplication>::run()
{
    if (!m_isNoRootForced) {
        this->allowRunAsRoot();
    }
    return tBaseApplication::run();
}

template <typename tBaseApplication>
int TestBaseApplication<tBaseApplication>::run(nf::testing::OnRunAction action)
{
    nf::testing::setOnRunAction(std::move(action));
    return run();
}

template <typename tBaseApplication>
int TestBaseApplication<tBaseApplication>::run(const std::function<void()> &action)
{
    return run([&](bool & /*isRunning*/) { action(); });
}

template <typename tBaseApplication>
void TestBaseApplication<tBaseApplication>::forceForbidRunAsRoot() noexcept
{
    m_isNoRootForced = true;
}

template <typename tBaseApplication>
void TestBaseApplication<tBaseApplication>::expectStartUpCalled() const
{
    EXPECT_EQ(1, cntStartUpCalled);
}

template <typename tBaseApplication>
void TestBaseApplication<tBaseApplication>::expectNoStartUpCalled() const
{
    EXPECT_EQ(0, cntStartUpCalled);
}

template <typename tBaseApplication>
void TestBaseApplication<tBaseApplication>::stop(int exitCode)
{
    tBaseApplication::stop(exitCode);
    EXPECT_EQ(1, cntShutDownCalled);
}

NF_END_NAMESPACE
