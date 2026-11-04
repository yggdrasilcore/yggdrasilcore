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

#ifndef _YCORE_CRASH_HANDLER_H_
#define _YCORE_CRASH_HANDLER_H_

#include "Define.h"
#include <cstddef>

namespace Ycore
{
    /**
     * @brief Stacks of one thread, as captured by the deadlock detector.
     */
    struct ThreadTrace
    {
        char const* name;
        long tid;
        unsigned long long busyMs;
        void* const* frames;
        int frameCount;
    };

    /**
     * @brief Writes a deadlock report for the given threads.
     *
     * The report goes to stderr and to a log file next to the crash reports. Unlike a
     * fatal signal report this does not terminate the process: a stalled operation may
     * still finish, so the server keeps running.
     */
    YC_COMMON_API void ReportDeadlock(char const* reason, ThreadTrace const* threads, std::size_t count);

    /**
     * @brief Installs handlers for fatal signals (SIGSEGV, SIGABRT, SIGBUS, SIGFPE, SIGILL).
     *
     * When one of them is raised, a symbolized backtrace of the crashing thread is written to
     * stderr (and therefore to `docker logs`) and to a file, after which the process dies from
     * the original signal, so the exit status and any core dump are preserved.
     *
     * Reports are written to the first of:
     *   - $YCORE_CRASH_LOG_DIR
     *   - $LOGS_DIR/crashes
     *   - $YC_LOGS_DIR/crashes
     *   - ./crashes
     *
     * Nothing is loaded or parsed at installation time, so this costs no startup time. Debug
     * information is only touched when a crash actually happens.
     *
     * On Windows this is a no-op; crash reporting is handled by WheatyExceptionReport.
     */
    YC_COMMON_API void InstallCrashHandler();
}

#endif // _YCORE_CRASH_HANDLER_H_
