/*
 * This file is part of the YggdrasilCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "CrashHandler.h"

#if YC_PLATFORM == YC_PLATFORM_WINDOWS

namespace Ycore
{
    void InstallCrashHandler()
    {
        // Windows crash reports are produced by WheatyExceptionReport.
    }
}

#else

#include <cerrno>
#include <climits>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <filesystem>
#include <string>
#include <system_error>

#if defined(__linux__)
#include <sys/syscall.h>
#include <ucontext.h>
#endif

#if defined(YC_HAVE_LIBDW) && defined(__linux__)
#  if defined(__has_include)
#    if __has_include(<elfutils/libdwfl.h>)
#      include <elfutils/libdwfl.h>
#    elif __has_include(<libdwfl.h>)
#      include <libdwfl.h>
#    endif
#  else
#    include <elfutils/libdwfl.h>
#  endif
#endif

// Set by Ycore::Assert/Fatal/Error/Abort in Errors.cpp, so the crash report can carry the
// message that triggered the crash.
extern "C" char const* YcoreAssertionFailedMessage;

namespace
{
    constexpr int MaxFrames = 128;
    constexpr int MaxLineLength = 1024;

    char g_crashLogDirectory[PATH_MAX] = {};
    char g_crashLogPath[PATH_MAX] = {};
    char g_alternateStack[512 * 1024] = {};

    // stderr first, crash log file second (if it could be opened).
    int g_outputDescriptors[2] = { STDERR_FILENO, -1 };

    volatile sig_atomic_t g_inHandler = 0;
    bool g_installed = false;

#if defined(YC_HAVE_LIBDW) && defined(__linux__)
    char* g_debugInfoPath = nullptr;
    Dwfl_Callbacks g_dwflCallbacks = {};
#endif

    void WriteAll(int descriptor, char const* data, size_t length)
    {
        while (length > 0)
        {
            ssize_t written = ::write(descriptor, data, length);
            if (written < 0)
            {
                if (errno == EINTR)
                    continue;

                return;
            }

            data += written;
            length -= static_cast<size_t>(written);
        }
    }

    /// Writes the same text to stderr (docker logs) and to the crash log file.
    void WriteBoth(char const* data, size_t length)
    {
        for (int descriptor : g_outputDescriptors)
        {
            if (descriptor >= 0)
                WriteAll(descriptor, data, length);
        }
    }

    void Report(char const* format, ...)
    {
        char buffer[MaxLineLength];

        va_list arguments;
        va_start(arguments, format);
        int length = std::vsnprintf(buffer, sizeof(buffer), format, arguments);
        va_end(arguments);

        if (length <= 0)
            return;

        size_t size = static_cast<size_t>(length);
        if (size >= sizeof(buffer))
            size = sizeof(buffer) - 1;

        WriteBoth(buffer, size);
    }

    void CurrentTimestamp(char* buffer, size_t size)
    {
        std::time_t now = std::time(nullptr);
        std::tm brokenDown = {};
        ::localtime_r(&now, &brokenDown);
        std::strftime(buffer, size, "%Y-%m-%d-%H-%M-%S", &brokenDown);
    }

    char const* SignalName(int signalNumber)
    {
        switch (signalNumber)
        {
            case SIGSEGV: return "SIGSEGV (segmentation fault)";
            case SIGABRT: return "SIGABRT (aborted)";
            case SIGBUS:  return "SIGBUS (bus error)";
            case SIGFPE:  return "SIGFPE (arithmetic exception)";
            case SIGILL:  return "SIGILL (illegal instruction)";
            default:      return "unknown fatal signal";
        }
    }

    void GetProcessName(char* buffer, size_t size)
    {
        buffer[0] = '\0';

#if defined(__linux__)
        int descriptor = ::open("/proc/self/comm", O_RDONLY | O_CLOEXEC);
        if (descriptor >= 0)
        {
            ssize_t length = ::read(descriptor, buffer, size - 1);
            ::close(descriptor);

            if (length > 0)
            {
                buffer[length] = '\0';
                if (char* newline = std::strchr(buffer, '\n'))
                    *newline = '\0';
            }
        }
#endif

        if (!buffer[0])
            std::snprintf(buffer, size, "unknown");
    }

    void OpenCrashLogFile(char const* timestamp)
    {
        g_outputDescriptors[1] = -1;
        g_crashLogPath[0] = '\0';

        if (!g_crashLogDirectory[0])
            return;

        std::snprintf(g_crashLogPath, sizeof(g_crashLogPath), "%s/crash-%s-pid%d.log",
            g_crashLogDirectory, timestamp, static_cast<int>(::getpid()));

        int descriptor = ::open(g_crashLogPath, O_WRONLY | O_CREAT | O_APPEND | O_CLOEXEC, 0644);
        if (descriptor >= 0)
            g_outputDescriptors[1] = descriptor;
        else
            g_crashLogPath[0] = '\0';
    }

    void WriteHeader(int signalNumber, siginfo_t const* info, char const* timestamp, char const* processName)
    {
        Report("\n===== YggdrasilCore crash report =====\n");
        Report("Time:     %s\n", timestamp);
        Report("Signal:   %s (%d)\n", SignalName(signalNumber), signalNumber);
        Report("Process:  %s (pid %d", processName, static_cast<int>(::getpid()));

#if defined(__linux__)
        Report(", tid %ld", static_cast<long>(::syscall(SYS_gettid)));
#endif

        Report(")\n");

        if (info && (signalNumber == SIGSEGV || signalNumber == SIGBUS || signalNumber == SIGILL))
        {
            Report("Fault:    address 0x%llx, code %d\n",
                static_cast<unsigned long long>(reinterpret_cast<std::uintptr_t>(info->si_addr)), info->si_code);
        }

        if (g_crashLogPath[0])
            Report("Log file: %s\n", g_crashLogPath);
        else
            Report("Log file: <unavailable, writing to stderr only>\n");

        if (YcoreAssertionFailedMessage && *YcoreAssertionFailedMessage)
            Report("Assert:   %s\n", YcoreAssertionFailedMessage);

        Report("--------------------------------------\n");
    }

    /// Module and offset of every frame. This needs no debug information and always works,
    /// so it still gives a usable traceback if DWARF symbolization fails or is unavailable.
    void WriteRawTrace(void* const* frames, int firstFrame, int count)
    {
        Report("Raw stack (module+offset, resolvable with addr2line):\n");

        if (count <= firstFrame)
        {
            Report("  <no frames captured>\n");
            return;
        }

        for (int index = firstFrame; index < count; ++index)
        {
            unsigned long long address = static_cast<unsigned long long>(reinterpret_cast<std::uintptr_t>(frames[index]));

            Dl_info info = {};
            if (::dladdr(frames[index], &info) != 0 && info.dli_fname)
            {
                unsigned long long base = static_cast<unsigned long long>(reinterpret_cast<std::uintptr_t>(info.dli_fbase));
                Report("  #%02d 0x%016llx %s+0x%llx\n", index - firstFrame, address, info.dli_fname, address - base);
            }
            else
                Report("  #%02d 0x%016llx <unknown module>\n", index - firstFrame, address);
        }
    }

#if defined(YC_HAVE_LIBDW) && defined(__linux__)
    /// Demangles a C++ symbol. The caller owns the returned buffer.
    char* TryDemangle(char const* symbol)
    {
        if (!symbol || !*symbol)
            return nullptr;

        int status = -1;
        char* demangled = abi::__cxa_demangle(symbol, nullptr, nullptr, &status);
        if (status != 0 || !demangled)
        {
            std::free(demangled);
            return nullptr;
        }

        return demangled;
    }
#endif

    void WriteSymbolizedTrace(void* const* frames, int firstFrame, int count)
    {
        Report("Symbolized stack:\n");

        if (count <= firstFrame)
        {
            Report("  <no frames captured>\n");
            return;
        }

#if defined(YC_HAVE_LIBDW) && defined(__linux__)
        g_dwflCallbacks.find_elf = dwfl_linux_proc_find_elf;
        g_dwflCallbacks.find_debuginfo = dwfl_standard_find_debuginfo;
        g_dwflCallbacks.debuginfo_path = &g_debugInfoPath;

        Dwfl* dwfl = dwfl_begin(&g_dwflCallbacks);
        if (!dwfl
            || dwfl_linux_proc_report(dwfl, ::getpid()) != 0
            || dwfl_report_end(dwfl, nullptr, nullptr) != 0)
        {
            Report("  <DWARF symbolizer unavailable, use the raw stack above>\n");

            if (dwfl)
                dwfl_end(dwfl);

            return;
        }

        for (int index = firstFrame; index < count; ++index)
        {
            Dwarf_Addr address = static_cast<Dwarf_Addr>(reinterpret_cast<std::uintptr_t>(frames[index]));
            Dwfl_Module* module = dwfl_addrmodule(dwfl, address);

            char const* symbol = module ? dwfl_module_addrname(module, address) : nullptr;
            char* demangled = TryDemangle(symbol);

            char location[MaxLineLength / 2] = {};
            Dwfl_Line* sourceLine = module ? dwfl_module_getsrc(module, address) : nullptr;
            if (sourceLine)
            {
                Dwarf_Addr lineAddress = 0;
                int lineNumber = 0;
                int column = 0;
                char const* file = dwfl_lineinfo(sourceLine, &lineAddress, &lineNumber, &column, nullptr, nullptr);

                if (file && lineNumber > 0)
                    std::snprintf(location, sizeof(location), " at %s:%d", file, lineNumber);
                else if (file)
                    std::snprintf(location, sizeof(location), " at %s", file);
            }

            if (demangled || symbol)
                Report("  #%02d 0x%016llx %s%s\n", index - firstFrame,
                    static_cast<unsigned long long>(address), demangled ? demangled : symbol, location);
            else
            {
                Dwarf_Addr start = 0;
                char const* moduleName = module
                    ? dwfl_module_info(module, nullptr, &start, nullptr, nullptr, nullptr, nullptr, nullptr)
                    : nullptr;

                if (moduleName)
                    Report("  #%02d 0x%016llx %s+0x%llx%s\n", index - firstFrame,
                        static_cast<unsigned long long>(address), moduleName,
                        static_cast<unsigned long long>(address - start), location);
                else
                    Report("  #%02d 0x%016llx ??%s\n", index - firstFrame,
                        static_cast<unsigned long long>(address), location);
            }

            std::free(demangled);
        }

        dwfl_end(dwfl);
#else
        (void)frames;
        (void)firstFrame;
        (void)count;
        Report("  <DWARF symbolizer unavailable, use the raw stack above>\n");
#endif
    }

    /// Program counter of the instruction that was executing when the signal arrived.
    /// backtrace() starts in this handler, so this is used to skip the handler and signal
    /// trampoline frames, and to still report the faulting instruction when unwinding fails
    /// (for example after a stack overflow).
    std::uintptr_t GetInterruptedProgramCounter(void* context)
    {
#if defined(__linux__) && defined(__x86_64__)
        if (!context)
            return 0;

        return static_cast<std::uintptr_t>(static_cast<ucontext_t*>(context)->uc_mcontext.gregs[REG_RIP]);
#else
        (void)context;
        return 0;
#endif
    }

    void HandleFatalSignal(int signalNumber, siginfo_t* info, void* context)
    {
        if (g_inHandler)
            _exit(128 + signalNumber);

        g_inHandler = 1;

        // Reset the disposition immediately, so a fault inside the handler kills the
        // process instead of re-entering it.
        struct sigaction defaultAction = {};
        defaultAction.sa_handler = SIG_DFL;
        ::sigemptyset(&defaultAction.sa_mask);
        ::sigaction(signalNumber, &defaultAction, nullptr);

        char timestamp[32] = {};
        CurrentTimestamp(timestamp, sizeof(timestamp));

        g_outputDescriptors[0] = STDERR_FILENO;
        OpenCrashLogFile(timestamp);

        char processName[64] = {};
        GetProcessName(processName, sizeof(processName));

        WriteHeader(signalNumber, info, timestamp, processName);

        void* frames[MaxFrames] = {};
        int frameCount = ::backtrace(frames, MaxFrames);

        std::uintptr_t programCounter = GetInterruptedProgramCounter(context);
        int firstFrame = 1;
        bool foundInterruptedFrame = false;

        for (int index = 1; index < frameCount; ++index)
        {
            if (reinterpret_cast<std::uintptr_t>(frames[index]) == programCounter)
            {
                firstFrame = index;
                foundInterruptedFrame = true;
                break;
            }
        }

        if (!foundInterruptedFrame && programCounter != 0 && frameCount < MaxFrames)
        {
            // The unwinder could not reach the interrupted frame; report it ourselves so the
            // faulting instruction is never lost.
            frames[frameCount++] = reinterpret_cast<void*>(programCounter);
            firstFrame = frameCount - 1;
        }

        WriteRawTrace(frames, firstFrame, frameCount);
        WriteSymbolizedTrace(frames, firstFrame, frameCount);

        Report("===== end of crash report =====\n");

        if (g_outputDescriptors[1] >= 0)
        {
            ::close(g_outputDescriptors[1]);
            g_outputDescriptors[1] = -1;
        }

        // Die from the original signal so the exit status (and any enabled core dump)
        // reflects the real failure.
        sigset_t unblocked;
        ::sigemptyset(&unblocked);
        ::sigaddset(&unblocked, signalNumber);
        ::sigprocmask(SIG_UNBLOCK, &unblocked, nullptr);
        ::raise(signalNumber);

        _exit(128 + signalNumber);
    }
}

void Ycore::InstallCrashHandler()
{
    if (g_installed)
        return;

    g_installed = true;

    std::string directory;
    if (char const* explicitDirectory = std::getenv("YCORE_CRASH_LOG_DIR"); explicitDirectory && *explicitDirectory)
        directory = explicitDirectory;
    else if (char const* logsDirectory = std::getenv("LOGS_DIR"); logsDirectory && *logsDirectory)
        directory = std::string(logsDirectory) + "/crashes";
    else if (char const* logsDirectory = std::getenv("YC_LOGS_DIR"); logsDirectory && *logsDirectory)
        directory = std::string(logsDirectory) + "/crashes";
    else
        directory = "crashes";

    std::error_code error;
    std::filesystem::create_directories(directory, error);
    std::snprintf(g_crashLogDirectory, sizeof(g_crashLogDirectory), "%s", directory.c_str());

    // Symbolization must never block on a debuginfod lookup.
    ::unsetenv("DEBUGINFOD_URLS");

    // Warm up everything that would otherwise be initialized lazily inside the handler:
    // the unwinder (backtrace) and the time zone data (localtime_r).
    void* warmup[2] = {};
    ::backtrace(warmup, 2);
    std::time_t now = std::time(nullptr);
    std::tm brokenDown = {};
    ::localtime_r(&now, &brokenDown);

    // Dedicated signal stack, so a stack overflow can still be reported.
    stack_t alternateStack = {};
    alternateStack.ss_sp = g_alternateStack;
    alternateStack.ss_size = sizeof(g_alternateStack);
    alternateStack.ss_flags = 0;
    ::sigaltstack(&alternateStack, nullptr);

    struct sigaction action = {};
    action.sa_sigaction = &HandleFatalSignal;
    action.sa_flags = SA_SIGINFO | SA_ONSTACK;
    ::sigfillset(&action.sa_mask);

    for (int signalNumber : { SIGSEGV, SIGABRT, SIGBUS, SIGFPE, SIGILL })
        ::sigaction(signalNumber, &action, nullptr);

    if (error)
        std::fprintf(stderr, "Crash report directory %s is not usable: %s\n", g_crashLogDirectory, error.message().c_str());
    else
        std::fprintf(stderr, "Crash reports will be written to %s\n", g_crashLogDirectory);
}

#endif
