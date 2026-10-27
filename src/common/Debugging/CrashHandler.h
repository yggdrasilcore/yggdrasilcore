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

namespace Ycore
{
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
