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

#include "DeadlockDetector.h"
#include "CrashHandler.h"

#if YC_PLATFORM != YC_PLATFORM_WINDOWS && defined(__linux__)

#include <atomic>
#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <signal.h>
#include <thread>

#include <execinfo.h>
#include <sys/syscall.h>
#include <ucontext.h>
#include <unistd.h>

namespace
{
    constexpr std::size_t MaxFrames = 128;
    constexpr std::size_t MaxTrackedThreads = 1 + 64; // world + up to 64 map workers
    constexpr std::size_t InvalidSlot = MaxTrackedThreads;

    // glibc reserves SIGRTMIN and SIGRTMIN + 1 for its own use.
    int const CaptureSignal = SIGRTMIN + 2;

    struct TrackedSlot
    {
        std::atomic<unsigned long long> busySinceMs{ 0 };
        std::atomic<long> tid{ 0 };
        std::atomic<int> frameCount{ 0 };
        std::atomic<bool> captured{ false };
        void* frames[MaxFrames] = {};
        char name[24] = {};
    };

    TrackedSlot g_slots[MaxTrackedThreads];
    thread_local std::size_t t_slot = InvalidSlot;
    std::atomic<bool> g_started{ false };

    unsigned long long NowMs()
    {
        return static_cast<unsigned long long>(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now().time_since_epoch()).count());
    }

    std::size_t SlotFor(Ycore::TrackedThread kind, uint32 index)
    {
        if (kind == Ycore::TrackedThread::World)
            return 0;

        return 1 + static_cast<std::size_t>(index);
    }

    /// Program counter of the instruction that was executing when the capture signal
    /// arrived, so the handler can skip its own frames.
    std::uintptr_t InterruptedProgramCounter(void* context)
    {
#if defined(__x86_64__)
        if (!context)
            return 0;

        return static_cast<std::uintptr_t>(static_cast<ucontext_t*>(context)->uc_mcontext.gregs[REG_RIP]);
#elif defined(__aarch64__)
        if (!context)
            return 0;

        return static_cast<std::uintptr_t>(static_cast<ucontext_t*>(context)->uc_mcontext.pc);
#else
        (void)context;
        return 0;
#endif
    }

    void CaptureHandler(int, siginfo_t*, void* context)
    {
        std::size_t slot = t_slot;
        if (slot >= MaxTrackedThreads)
            return;

        void* frames[MaxFrames];
        int count = ::backtrace(frames, static_cast<int>(MaxFrames));

        std::uintptr_t programCounter = InterruptedProgramCounter(context);
        if (programCounter != 0)
        {
            int firstFrame = 0;
            for (int index = 1; index < count; ++index)
            {
                if (reinterpret_cast<std::uintptr_t>(frames[index]) == programCounter)
                {
                    firstFrame = index;
                    break;
                }
            }

            if (firstFrame > 0)
            {
                count -= firstFrame;
                std::memmove(frames, frames + firstFrame, sizeof(void*) * static_cast<std::size_t>(count));
            }
            else if (count < static_cast<int>(MaxFrames))
            {
                std::memmove(frames + 1, frames, sizeof(void*) * static_cast<std::size_t>(count));
                frames[0] = reinterpret_cast<void*>(programCounter);
                ++count;
            }
        }

        TrackedSlot& tracked = g_slots[slot];
        std::memcpy(tracked.frames, frames, sizeof(void*) * static_cast<std::size_t>(count));
        tracked.frameCount.store(count, std::memory_order_release);
        tracked.captured.store(true, std::memory_order_release);
    }

    void CaptureThread(std::size_t slot)
    {
        TrackedSlot& tracked = g_slots[slot];

        tracked.captured.store(false, std::memory_order_relaxed);
        tracked.frameCount.store(0, std::memory_order_relaxed);

        long tid = tracked.tid.load(std::memory_order_acquire);
        if (tid <= 0)
            return;

        ::syscall(SYS_tgkill, static_cast<pid_t>(::getpid()), static_cast<pid_t>(tid), CaptureSignal);

        // A thread stuck in an uninterruptible syscall will not answer in time; the
        // report then shows it without frames instead of waiting forever.
        for (int attempt = 0; attempt < 250; ++attempt)
        {
            if (tracked.captured.load(std::memory_order_acquire))
                return;

            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }

    void LogStall(std::size_t stuckSlot, unsigned long long elapsedMs)
    {
        TrackedSlot const& stuck = g_slots[stuckSlot];

        char reason[256] = {};
        std::snprintf(reason, sizeof(reason), "thread '%s' (tid %ld) has been in a single operation for %llu ms",
            stuck.name, stuck.tid.load(std::memory_order_acquire), elapsedMs);

        unsigned long long now = NowMs();

        Ycore::ThreadTrace traces[MaxTrackedThreads] = {};
        std::size_t count = 0;

        for (std::size_t slot = 0; slot < MaxTrackedThreads; ++slot)
        {
            TrackedSlot& tracked = g_slots[slot];

            long tid = tracked.tid.load(std::memory_order_acquire);
            if (tid <= 0)
                continue;

            CaptureThread(slot);

            unsigned long long since = tracked.busySinceMs.load(std::memory_order_acquire);

            traces[count].name = tracked.name;
            traces[count].tid = tid;
            traces[count].busyMs = since == 0 ? 0 : now - since;
            traces[count].frames = tracked.frames;
            traces[count].frameCount = tracked.frameCount.load(std::memory_order_acquire);
            ++count;
        }

        Ycore::ReportDeadlock(reason, traces, count);
    }

    void WatchdogThread(Milliseconds timeout)
    {
        unsigned long long const timeoutMs = static_cast<unsigned long long>(timeout.count());
        unsigned long long lastLoggedSince[MaxTrackedThreads] = {};

        for (;;)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));

            unsigned long long now = NowMs();

            for (std::size_t slot = 0; slot < MaxTrackedThreads; ++slot)
            {
                unsigned long long since = g_slots[slot].busySinceMs.load(std::memory_order_acquire);
                if (since == 0 || now - since < timeoutMs)
                    continue;

                // Log each operation once; it may still finish and be followed by another.
                if (lastLoggedSince[slot] == since)
                    continue;

                lastLoggedSince[slot] = since;
                LogStall(slot, now - since);
            }
        }
    }
}

Ycore::TrackedOperation::TrackedOperation(TrackedThread kind, uint32 index)
    : _slot(static_cast<uint32>(InvalidSlot))
{
    std::size_t slot = SlotFor(kind, index);
    if (slot >= MaxTrackedThreads)
        return;

    _slot = static_cast<uint32>(slot);

    if (t_slot != slot)
    {
        t_slot = slot;

        TrackedSlot& tracked = g_slots[slot];
        tracked.tid.store(static_cast<long>(::syscall(SYS_gettid)), std::memory_order_release);

        if (kind == TrackedThread::World)
            std::snprintf(tracked.name, sizeof(tracked.name), "world");
        else
            std::snprintf(tracked.name, sizeof(tracked.name), "map-%u", static_cast<unsigned>(index));

        // Warm up the unwinder on this thread, so the capture handler never has to
        // initialize it while the process is already stuck.
        void* warmup[2] = {};
        ::backtrace(warmup, 2);
    }

    g_slots[slot].busySinceMs.store(NowMs(), std::memory_order_release);
}

Ycore::TrackedOperation::~TrackedOperation()
{
    if (_slot < MaxTrackedThreads)
        g_slots[_slot].busySinceMs.store(0, std::memory_order_release);
}

void Ycore::StartDeadlockDetector(Milliseconds timeout)
{
    if (g_started.exchange(true))
        return;

    struct sigaction action = {};
    action.sa_sigaction = &CaptureHandler;
    action.sa_flags = SA_SIGINFO | SA_RESTART;
    ::sigemptyset(&action.sa_mask);
    ::sigaction(CaptureSignal, &action, nullptr);

    std::thread(WatchdogThread, timeout).detach();
}

#else

namespace Ycore
{
    TrackedOperation::TrackedOperation(TrackedThread, uint32) : _slot(0) { }

    TrackedOperation::~TrackedOperation() = default;

    void StartDeadlockDetector(Milliseconds) { }
}

#endif
