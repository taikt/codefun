/*
 * BMW Neo Framework
 *
 * Copyright (C) 2015-2016, BMW AG
 * Copyright (C) 2015-2020 BMW Car IT GmbH. All rights reserved.
 * Contact: http://www.bmw-carit.de/
 *
 * This code is based on NaRE and is reused from ascgit125.nare repository.
 *
 * Contributors:
 *     Benoît Sauts <benoit.sauts@bmw-carit.de>
 *     Manuel Nickschas <manuel.nickschas@bmw-carit.de>
 *     Monika Kistler <monika.kistler@bmw-carit.de>
 *     Mikołaj Piekło <mikolaj.pieklo@partner.bmw.de>
 */

#pragma once

#include <nf/Enums.h>

#include <csignal>
#include <functional>

/*
 * glibc has removed SIGUNUSED as obsolete, therefore it is defined here, as a workaround.
 * Value copied from /usr/include/asm/signal.h
 */
#if !defined(SIGUNUSED)
#define SIGUNUSED 31
#endif

NF_BEGIN_NAMESPACE

/**
 * @ingroup nf_core_Executors
 * @defgroup nf_core_LinuxSignals Linux Signals
 *
 * The NF Linux signal handling mechanism.
 *
 * @{
 * @par Introduction
 *
 * This code is based on NaRE and is reused from <a
 href="https://asc-repo.bmwgroup.net/gerrit/ascgit125.nare">NaRe repository</a>.
 *
 * @see @nfref{Executor::setSignalHandlers()}
 *
 * NF provides support for Linux signals and covers the following points:
 *
 * - Propagates the signals to the main loop,
 * - Default signal handling is used in case the users do not define their own callback,
 * - Supports custom signal handling, which can be either passed as a function pointer, an
 *   std::function or a lambda,
 *
 * @note This implementation does not support waiting for signals, as this does not conceptually
 *       work with handling the signals in the main event loop i.e. one shall not block the main
 *       event loop to wait for a signal.
 *
 * @par Reentrancy
 *
 * A well-known limitation when handling signals, is that one should normally only use so-called
 * reentrant functions within the signal handler.
 *
 * However, this limitation doesn't affect the NF signal handling implementation, which means
 * that you are free to use any function you desire within your signal handler.
 *
 * @par Setting-up Signal Handling
 *

 *
 * @warning To ensure that the signal handling works properly, it is mandatory to setup the signal
 *          handling before anything else you do which might spawn a thread.@n
 *          As the signal mask is inherited from the main thread by the children threads, if a
 *          thread is created before the mask is set, that thread will have the default signal mask,
 *          hence it will not block any signals. Unfortunately, the Linux kernel randomly passes the
 *          signal to a thread in the process. Therefore it may happen in such case that the signal
 *          is not handled.@n
 *          Since NF has no way to access the threads created before the signal mask has been set,
 *          we request the user to actually take care of setting the signal handling first.
 */

/**
 * LinuxSignal maps the POSIX reliable signals to an enum class to trigger compile-time errors
 * when trying to create a signal which is not valid.
 *
 * @note SIGPWR and SIGWINCH are not specified in the POSIX specification.
 *       They are, however, sometimes used on Linux.
 *
 * @note SIGKILL and SIGSTOP are omitted on purpose as these signals cannot be caught.
 *
 * @todo    If real time signals are required, add them.
 */
enum class LinuxSignal : int
{
    SigHup = SIGHUP,    ///< Hangup detected on controlling terminal or death of controlling process
    SigInt = SIGINT,    ///< Interrupt from keyboard
    SigQuit = SIGQUIT,  ///< Quit from keyboard
    SigIll = SIGILL,    ///< Illegal instruction
    SigTrap = SIGTRAP,  ///< Trace/breakpoint trap
    SigAbort = SIGABRT, ///< Abort signal from abort()
    SigBus = SIGBUS,    ///< Bus error (bad memory access)
    SigFpe = SIGFPE,    ///< Floating point exception
    SigUsr1 = SIGUSR1,  ///< User-defined signal 1
    SigSegv = SIGSEGV,  ///< Segmentation violation (invalid memory reference)
    SigUsr2 = SIGUSR2,  ///< User-defined signal 2
    SigPipe = SIGPIPE,  ///< Broken pipe: write to pipe with no readers
    SigAlrm = SIGALRM,  ///< Timer signal from alarm()
    SigTerm = SIGTERM,  ///< Termination signal
    SigStkflt = SIGSTKFLT, ///< Stack fault on coprocessor (unused)
    SigChld = SIGCHLD,     ///< Child stopped or terminated
    SigCont = SIGCONT,     ///< Continue if stopped
    SigTstp = SIGTSTP,     ///< Stop typed at terminal
    SigTtin = SIGTTIN,     ///< Terminal input for background process
    SigTtou = SIGTTOU,     ///< Terminal output for background process
    SigUrg = SIGURG,       ///< Urgent condition on socket
    SigXcpu = SIGXCPU,     ///< CPU time limit exceeded
    SigXfsz = SIGXFSZ,     ///< File size limit exceeded
    SigVtAlrm = SIGVTALRM, ///< Virtual alarm clock
    SigProf = SIGPROF,     ///< Profiling timer expired
    SigWinch = SIGWINCH,   ///< Window resize signal
    SigPoll = SIGPOLL,     ///< Pollable event occurred, a synonym for SIGIO
    SigPwr = SIGPWR,       ///< Power failure restart (System V)
    SigSys = SIGSYS,       ///< Bad system call
};

NF_ENUM_DEFINE_UTILS(LinuxSignal, SigHup, SigInt, SigQuit, SigIll, SigTrap, SigAbort, SigBus,
                     SigFpe, SigUsr1, SigSegv, SigUsr2, SigPipe, SigAlrm, SigTerm, SigStkflt,
                     SigChld, SigCont, SigTstp, SigTtin, SigTtou, SigUrg, SigXcpu, SigXfsz,
                     SigVtAlrm, SigProf, SigWinch, SigPoll, SigPwr, SigSys)

/**
 * Typedef to be able to enforce the callback signature and to make sure that lambdas
 * can be used directly as callbacks for the signal. This Callback accepts @e LinuxSignal as param.
 */
using LinuxSignalHandlerCallback = std::function<void(const LinuxSignal &signal)>;

/**
 * An LinuxSignalHandler is a struct composed of a signal number and an LinuxSignalHandlerCallback.
 *
 * Upon arrival of the given signal, the provided callback function will be executed. Information
 * about the signal is available as parameter given to the callback.
 */
struct LinuxSignalHandler
{
    LinuxSignal signal;
    LinuxSignalHandlerCallback cb;
};

/// @}

NF_END_NAMESPACE
