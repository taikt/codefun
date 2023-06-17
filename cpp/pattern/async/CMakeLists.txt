##
## BMW Neo Framework
##
## Copyright (C) 2018-2022 BMW Car IT GmbH. All rights reserved.
## Contact: http://www.bmw-carit.de/
##
## Contributors:
##     Antons Jeļkins <antons.jelkins@bmw.de>
##     Marko Popović <marko.popovic@bmw.de>
##     Manuel Nickschas <manuel.nickschas@bmw.de>
##

# Required depdendencies
nf_find_dependency(fmt VERSION 8 REQUIRED)
nf_find_dependency(libsystemd PKGCONFIG REQUIRED)
nf_find_dependency(Threads REQUIRED)

# Optional dependencies
nf_find_dependency(automotive-dlt PKGCONFIG
                   FOUND_FLAG NF_HAS_DLT
                   PURPOSE "Support logging to DLT"
)

nf_define_component(Core
  TEMPLATE CoreConfig.cmake.in
  DESCRIPTION "provides core functionality for the Neo Framework"
)

nf_define_module(
  NAME
    Core

  EXPORT_COMPONENT
    Core

  BUILD_FOR
    BOSP

  PUBLIC_DEPS
    Boost::boost
    Boost::program_options
    fmt::fmt

  PRIVATE_DEPS
    Boost::exception
    Boost::system
    $<$<BOOL:${NF_HAS_DLT}>:PkgConfig::automotive-dlt>
    PkgConfig::libsystemd
    Threads::Threads  # for Boost::asio

  PRIVATE_DEFINITIONS
    $<$<BOOL:${NF_HAS_DLT}>:NF_HAS_DLT>

  LOG_CONTEXT
    nf_core NFCO

  SOURCES
    $<$<BOOL:${NF_HAS_DLT}>:include/nf/backend/DltLogger.h>
    $<$<BOOL:${NF_HAS_DLT}>:src/nf/backend/DltLogger.cpp>
    include/nf/ApplicationOptions.h
    include/nf/AsioIoService.h
    include/nf/Assert.h
    include/nf/Async.h
    include/nf/Attribute.h
    include/nf/BaseApplication.h
    include/nf/ByteArray.h
    include/nf/ChronoIoStream.h
    include/nf/Composition.h
    include/nf/ContainerAlgorithms.h
    include/nf/ContainerIoStream.h
    include/nf/Context.h
    include/nf/ContextItem.h
    include/nf/DataSize.h
    include/nf/Demangle.h
    include/nf/Enums.h
    include/nf/Executor.h
    include/nf/Framework.h
    include/nf/Function.h
    include/nf/FunctionTraits.h
    include/nf/Future.h
    include/nf/IoWatch.h
    include/nf/LinuxSignal.h
    include/nf/Logging.h
    include/nf/MulticastPromise.h
    include/nf/OptionalIoStream.h
    include/nf/Printable.h
    include/nf/Promise.h
    include/nf/RaiiToken.h
    include/nf/RwLock.h
    include/nf/Signal.h
    include/nf/Singleton.h
    include/nf/Subscription.h
    include/nf/SuspendableLogger.h
    include/nf/Thread.h
    include/nf/Timer.h
    include/nf/TypeTraits.h
    include/nf/Version.h
    include/nf/async/Execute.h
    include/nf/backend/AsioExecutor.h
    include/nf/backend/MemoryLogger.h
    include/nf/backend/StdoutLogger.h
    include/nf/cpp20/Chrono.h
    include/nf/detail/AbstractUnit.h
    include/nf/detail/AnyFunction.h
    include/nf/detail/AsyncSharedState.h
    include/nf/detail/CallbackScope.h
    include/nf/detail/ContextState.h
    include/nf/detail/MulticastSharedState.h
    include/nf/detail/SignalBase.h
    src/nf/ApplicationOptions.cpp
    src/nf/AsioIoService.cpp
    src/nf/Assert.cpp
    src/nf/BaseApplication.cpp
    src/nf/ByteArray.cpp
    src/nf/Context.cpp
    src/nf/ContextItem.cpp
    src/nf/DataSize.cpp
    src/nf/Demangle.cpp
    src/nf/Executor.cpp
    src/nf/Framework.cpp
    src/nf/Function.cpp
    src/nf/IoWatch.cpp
    src/nf/Logging.cpp
    src/nf/Printable.cpp
    src/nf/RaiiToken.cpp
    src/nf/RwLock.cpp
    src/nf/Singleton.cpp
    src/nf/Subscription.cpp
    src/nf/SuspendableLogger.cpp
    src/nf/Thread.cpp
    src/nf/Timer.cpp
    src/nf/Version.cpp
    src/nf/backend/AsioExecutor.cpp
    src/nf/backend/AsioIoWatch.cpp
    src/nf/backend/AsioTimer.cpp
    src/nf/backend/MemoryLogger.cpp
    src/nf/backend/StdoutLogger.cpp
    src/nf/detail/CallbackScope.cpp
    src/nf/detail/ContextState.cpp
    src/nf/detail/SignalBase.cpp

  TEST_SOURCES
    test/cpp20/test_Chrono.cpp
    test/test_Assert.cpp
    test/test_Async.cpp
    test/test_Attribute.cpp
    test/test_BaseApplication.cpp
    test/test_ByteArray.cpp
    test/test_CallbackScope.cpp
    test/test_ChronoIoStream.cpp
    test/test_ContainerAlgorithms.cpp
    test/test_ContainerIoStream.cpp
    test/test_Context.cpp
    test/test_ContextItem.cpp
    test/test_DataSize.cpp
    test/test_Demangle.cpp
    test/test_Enums.cpp
    test/test_Function.cpp
    test/test_FunctionTraits.cpp
    test/test_Logging.cpp
    test/test_OptionalIoStream.cpp
    test/test_Printable.cpp
    test/test_RaiiToken.cpp
    test/test_Result.cpp
    test/test_RwLock.cpp
    test/test_Signal.cpp
    test/test_Singleton.cpp
    test/test_SuspendableLogger.cpp
    test/test_Thread.cpp
    test/test_Version.cpp

  MOCK_SOURCES
    mock/nf/TestBaseApplication.h

  DOCUMENTATION
    doc/Async.dox
    doc/BaseApplication.dox
    doc/Core.dox
    doc/ExecutionFlow.dox
    doc/Logging.dox
    doc/Signals.dox
    doc/Types.dox
)

nf_return_if_no_target()

nf_add_build_failure_tests(
  TARGET
    ${TARGET}
  SOURCES
    test/bft_Function.cpp*3
    test/bft_ByteArray.cpp*12
)

configure_file(${NF_SOURCE_DIR}/cmake/in/Global.h.in include/nf/Global.h @ONLY)
target_include_directories(${TARGET}
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
)
target_compile_options(${TARGET}
  PUBLIC
    -Wno-error=deprecated-declarations
)
install(
  FILES
    ${CMAKE_CURRENT_BINARY_DIR}/include/nf/Global.h
  DESTINATION
    ${NF_INSTALL_INCLUDEDIR}/${TARGET}/nf
)

nf_make_version_h(${TARGET}
  DESCRIPTION
    "Neo Framework"
  NAMESPACE
    nf
  PREFIX
    Nf
  INSTALL_DIR
    ${NF_INSTALL_INCLUDEDIR}/${TARGET}
)
