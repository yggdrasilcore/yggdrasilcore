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
#  include <dwarf.h>
#endif

// Set by Ycore::Assert/Fatal/Error/Abort in Errors.cpp, so the crash report can carry the
// message that triggered the crash.
extern "C" char const* YcoreAssertionFailedMessage;

namespace
{
    constexpr int MaxFrames = 128;
    constexpr int MaxLineLength = 1024;
    constexpr int MaxSourceLineLength = 400;
    constexpr int SourceContextLines = 2;
    constexpr int SourceContextFrames = 3;

    char g_crashLogDirectory[PATH_MAX] = {};
    char g_crashLogPath[PATH_MAX] = {};
    char g_alternateStack[512 * 1024] = {};

    // stderr first, crash log file second (if it could be opened).
    int g_outputDescriptors[2] = { STDERR_FILENO, -1 };

    char g_sourceRoot[PATH_MAX] = {};
    char g_frameDescription[2][MaxLineLength / 2] = {};
    char g_faultVariable[128] = {};
    void* g_faultContext = nullptr;
    std::uintptr_t g_faultAddress = 0;
    bool g_haveSourceLocations = false;
    bool g_sourceNoteShown = false;

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

    void WriteSourceContext(char const* file, int lineNumber)
    {
        if (!file || lineNumber <= 0)
            return;

        int descriptor = ::open(file, O_RDONLY | O_CLOEXEC);
        if (descriptor < 0 && g_sourceRoot[0])
        {
            char path[PATH_MAX];
            std::snprintf(path, sizeof(path), "%s/%s", g_sourceRoot, file);
            descriptor = ::open(path, O_RDONLY | O_CLOEXEC);
        }

        if (descriptor < 0)
        {
            if (!g_sourceNoteShown)
            {
                g_sourceNoteShown = true;
                Report("      <source not readable at %s, set YCORE_SOURCE_ROOT to the source checkout>\n", file);
            }

            return;
        }

        int const first = lineNumber > SourceContextLines ? lineNumber - SourceContextLines : 1;
        int const last = lineNumber + SourceContextLines;

        char chunk[4096];
        char line[MaxSourceLineLength] = {};
        size_t lineLength = 0;
        int current = 1;
        bool done = false;

        while (!done)
        {
            ssize_t received = ::read(descriptor, chunk, sizeof(chunk));
            if (received < 0)
            {
                if (errno == EINTR)
                    continue;

                break;
            }

            if (received == 0)
                break;

            for (ssize_t index = 0; index < received; ++index)
            {
                char character = chunk[index];
                if (character == '\n')
                {
                    if (current >= first)
                    {
                        line[lineLength] = '\0';
                        Report("      %c %5d | %s\n", current == lineNumber ? '>' : ' ', current, line);
                    }

                    ++current;
                    lineLength = 0;

                    if (current > last)
                    {
                        done = true;
                        break;
                    }
                }
                else if (character != '\r' && current >= first && lineLength + 1 < sizeof(line))
                    line[lineLength++] = character;
            }
        }

        ::close(descriptor);
    }

    bool GetFaultingBaseRegister(int* registerNumber, std::uint64_t* registerValue)
    {
#if defined(__linux__) && defined(__aarch64__)
        if (!g_faultContext)
            return false;

        ucontext_t const* context = static_cast<ucontext_t const*>(g_faultContext);
        std::uintptr_t programCounter = static_cast<std::uintptr_t>(context->uc_mcontext.pc);

        Dl_info moduleInfo = {};
        if (programCounter == g_faultAddress || ::dladdr(reinterpret_cast<void const*>(programCounter), &moduleInfo) == 0)
            return false;

        std::uint32_t instruction = 0;
        std::memcpy(&instruction, reinterpret_cast<void const*>(programCounter), sizeof(instruction));

        bool const isLoadStore = (instruction & 0x0A000000u) == 0x08000000u;
        bool const isLiteral = (instruction & 0x3B000000u) == 0x18000000u;
        if (!isLoadStore || isLiteral)
            return false;

        int number = static_cast<int>((instruction >> 5) & 31u);
        *registerNumber = number;
        *registerValue = number == 31 ? context->uc_mcontext.sp : context->uc_mcontext.regs[number];
        return true;
#else
        (void)registerNumber;
        (void)registerValue;
        return false;
#endif
    }

#if defined(YC_HAVE_LIBDW) && defined(__linux__)
    bool FindVariableInRegister(Dwarf_Die* scopes, int scopeCount, Dwarf_Addr pc, int dwarfRegister, char* name, size_t size)
    {
        for (int scopeIndex = 0; scopeIndex < scopeCount; ++scopeIndex)
        {
            Dwarf_Die child;
            if (dwarf_child(&scopes[scopeIndex], &child) != 0)
                continue;

            do
            {
                int tag = dwarf_tag(&child);
                if (tag != DW_TAG_variable && tag != DW_TAG_formal_parameter)
                    continue;

                Dwarf_Attribute location;
                if (!dwarf_attr_integrate(&child, DW_AT_location, &location))
                    continue;

                Dwarf_Op* expressions[1] = {};
                size_t lengths[1] = {};
                if (dwarf_getlocation_addr(&location, pc, expressions, lengths, 1) < 1 || lengths[0] != 1)
                    continue;

                if (expressions[0][0].atom != DW_OP_reg0 + dwarfRegister)
                    continue;

                char const* variableName = dwarf_diename(&child);
                if (!variableName)
                    continue;

                std::snprintf(name, size, "%s", variableName);
                return true;
            } while (dwarf_siblingof(&child, &child) == 0);
        }

        return false;
    }

    void WriteInlinedFrames(Dwarf_Die* compilationUnit, Dwarf_Die* scopes, int scopeCount)
    {
        Dwarf_Files* files = nullptr;
        size_t fileCount = 0;
        bool filesLoaded = false;

        for (int index = 0; index < scopeCount; ++index)
        {
            if (dwarf_tag(&scopes[index]) != DW_TAG_inlined_subroutine)
                continue;

            char const* name = dwarf_diename(&scopes[index]);

            Dwarf_Attribute attribute;
            Dwarf_Word callFile = 0;
            Dwarf_Word callLine = 0;
            bool const hasCallSite = dwarf_formudata(dwarf_attr(&scopes[index], DW_AT_call_file, &attribute), &callFile) == 0
                && dwarf_formudata(dwarf_attr(&scopes[index], DW_AT_call_line, &attribute), &callLine) == 0;

            if (hasCallSite && !filesLoaded)
            {
                filesLoaded = true;
                if (dwarf_getsrcfiles(compilationUnit, &files, &fileCount) != 0)
                    files = nullptr;
            }

            char const* callFileName = hasCallSite && files ? dwarf_filesrc(files, static_cast<size_t>(callFile), nullptr, nullptr) : nullptr;

            if (callFileName)
                Report("      inlined %s, called at %s:%llu\n", name ? name : "??", callFileName, static_cast<unsigned long long>(callLine));
            else
                Report("      inlined %s\n", name ? name : "??");
        }
    }

    void WriteFrameDetails(Dwfl_Module* module, Dwarf_Addr lookup, bool isFaultFrame, char const* file, int lineNumber, bool showSource)
    {
        Dwarf_Addr bias = 0;
        Dwarf_Die* compilationUnit = dwfl_module_addrdie(module, lookup, &bias);
        if (compilationUnit)
        {
            Dwarf_Die* scopes = nullptr;
            int scopeCount = dwarf_getscopes(compilationUnit, lookup - bias, &scopes);
            if (scopeCount > 0)
            {
                WriteInlinedFrames(compilationUnit, scopes, scopeCount);

                int registerNumber = 0;
                std::uint64_t registerValue = 0;
                if (isFaultFrame && GetFaultingBaseRegister(&registerNumber, &registerValue))
                    FindVariableInRegister(scopes, scopeCount, lookup - bias, registerNumber, g_faultVariable, sizeof(g_faultVariable));
            }

            std::free(scopes);
        }

        if (showSource)
            WriteSourceContext(file, lineNumber);
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
            Dwarf_Addr lookup = index == firstFrame ? address : address - 1;
            Dwfl_Module* module = dwfl_addrmodule(dwfl, lookup);

            char const* symbol = module ? dwfl_module_addrname(module, lookup) : nullptr;
            char* demangled = TryDemangle(symbol);

            char location[MaxLineLength / 2] = {};
            char const* file = nullptr;
            int lineNumber = 0;
            Dwfl_Line* sourceLine = module ? dwfl_module_getsrc(module, lookup) : nullptr;
            if (sourceLine)
            {
                Dwarf_Addr lineAddress = 0;
                int column = 0;
                file = dwfl_lineinfo(sourceLine, &lineAddress, &lineNumber, &column, nullptr, nullptr);

                if (file && lineNumber > 0)
                {
                    std::snprintf(location, sizeof(location), " at %s:%d", file, lineNumber);
                    g_haveSourceLocations = true;
                }
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

            if (index - firstFrame < 2)
                std::snprintf(g_frameDescription[index - firstFrame], sizeof(g_frameDescription[0]), "%s%s",
                    demangled ? demangled : (symbol ? symbol : "??"), location);

            if (module)
                WriteFrameDetails(module, lookup, index == firstFrame, file, lineNumber, index - firstFrame < SourceContextFrames);

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
#elif defined(__linux__) && defined(__aarch64__)
        if (!context)
            return 0;

        return static_cast<std::uintptr_t>(static_cast<ucontext_t*>(context)->uc_mcontext.pc);
#else
        (void)context;
        return 0;
#endif
    }

    std::uintptr_t GetInterruptedStackPointer(void* context)
    {
#if defined(__linux__) && defined(__x86_64__)
        if (!context)
            return 0;

        return static_cast<std::uintptr_t>(static_cast<ucontext_t*>(context)->uc_mcontext.gregs[REG_RSP]);
#elif defined(__linux__) && defined(__aarch64__)
        if (!context)
            return 0;

        return static_cast<std::uintptr_t>(static_cast<ucontext_t*>(context)->uc_mcontext.sp);
#else
        (void)context;
        return 0;
#endif
    }

    void WriteAnalysis(int signalNumber, siginfo_t const* info)
    {
        Report("Analysis:\n");

        if (g_frameDescription[0][0])
            Report("  Crashed in:  %s\n", g_frameDescription[0]);

        if (g_frameDescription[1][0])
            Report("  Called from: %s\n", g_frameDescription[1]);

#if defined(YC_HAVE_LIBDW) && defined(__linux__)
        if (!g_haveSourceLocations)
            Report("  No file:line resolved. Check that the binary has debug info and .debug_aranges (clang needs -gdwarf-aranges).\n");
#endif

        std::uintptr_t const address = info ? reinterpret_cast<std::uintptr_t>(info->si_addr) : 0;
        int const code = info ? info->si_code : 0;
        unsigned long long const printableAddress = static_cast<unsigned long long>(address);

        int baseRegister = 0;
        std::uint64_t baseValue = 0;
        bool const haveBase = signalNumber == SIGSEGV && GetFaultingBaseRegister(&baseRegister, &baseValue);
        if (haveBase)
        {
            if (baseRegister == 31)
                Report("  Register:    sp = 0x%llx", static_cast<unsigned long long>(baseValue));
            else
                Report("  Register:    x%d = 0x%llx", baseRegister, static_cast<unsigned long long>(baseValue));

            if (g_faultVariable[0])
                Report(" (variable '%s')", g_faultVariable);

            Report("\n");
        }

        switch (signalNumber)
        {
            case SIGSEGV:
            {
                std::uintptr_t const stackPointer = GetInterruptedStackPointer(g_faultContext);
                std::uintptr_t const distance = address > stackPointer ? address - stackPointer : stackPointer - address;

                if (address < 65536)
                {
                    Report("  Likely:      Null pointer dereference, offset 0x%llx from a null pointer was accessed.\n", printableAddress);

                    if (std::strcmp(g_faultVariable, "this") == 0)
                        Report("  Detail:      The method ran on a null object, so the caller passed or held a null pointer (see Called from).\n");
                    else if (g_faultVariable[0])
                        Report("  Detail:      '%s' is null.\n", g_faultVariable);
                    else
                        Report("  Detail:      A pointer in the crashing function is null, check the source lines above.\n");
                }
                else if (stackPointer != 0 && distance < 65536)
                    Report("  Likely:      Stack overflow, usually runaway recursion. The fault address is next to the stack pointer.\n");
                else if (code == SEGV_ACCERR)
                    Report("  Likely:      Access to mapped memory that does not allow it, such as a write to read-only data or a corrupted pointer or vtable.\n");
                else
                    Report("  Likely:      Access to unmapped memory at 0x%llx, a dangling pointer (use after free), an uninitialized pointer or corrupted memory.\n", printableAddress);

                break;
            }

            case SIGABRT:
                if (YcoreAssertionFailedMessage && *YcoreAssertionFailedMessage)
                    Report("  Likely:      A server assertion failed, see the Assert line above.\n");
                else
                    Report("  Likely:      abort() without an assertion message: an uncaught exception, std::terminate, or glibc detecting heap corruption or a double free.\n");

                break;

            case SIGFPE:
                if (code == FPE_INTDIV)
                    Report("  Likely:      Integer division or modulo by zero.\n");
                else if (code == FPE_INTOVF)
                    Report("  Likely:      Integer overflow trap.\n");
                else
                    Report("  Likely:      Arithmetic exception, such as a floating point division by zero.\n");

                break;

            case SIGILL:
                Report("  Likely:      Illegal instruction, a jump into data through a corrupted function pointer or vtable, or a binary built for a different CPU.\n");
                break;

            case SIGBUS:
                if (code == BUS_ADRALN)
                    Report("  Likely:      Misaligned memory access.\n");
                else
                    Report("  Likely:      Access to memory that has no backing, such as a truncated or removed memory mapped file.\n");

                break;

            default:
                break;
        }
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

        g_faultContext = context;
        g_faultAddress = info ? reinterpret_cast<std::uintptr_t>(info->si_addr) : 0;

        WriteRawTrace(frames, firstFrame, frameCount);
        WriteSymbolizedTrace(frames, firstFrame, frameCount);
        WriteAnalysis(signalNumber, info);

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

    if (char const* sourceRoot = std::getenv("YCORE_SOURCE_ROOT"); sourceRoot && *sourceRoot)
        std::snprintf(g_sourceRoot, sizeof(g_sourceRoot), "%s", sourceRoot);

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
