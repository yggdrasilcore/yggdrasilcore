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

#ifndef _YCORE_DEADLOCK_DETECTOR_H_
#define _YCORE_DEADLOCK_DETECTOR_H_

#include "Define.h"
#include "Duration.h"

namespace Ycore
{
    /// Threads that can introduce a deadlock, because they are the ones taking locks.
    enum class TrackedThread : uint8
    {
        World,
        MapWorker
    };

    /**
     * @brief Marks the current operation of a tracked thread as in progress.
     *
     * While an instance exists the watchdog considers the thread busy. Construct one
     * around the unit of work (a world tick, a map update request) so a thread that is
     * waiting for work is not mistaken for a stuck one.
     */
    class YC_COMMON_API TrackedOperation
    {
    public:
        TrackedOperation(TrackedThread kind, uint32 index);
        ~TrackedOperation();

        TrackedOperation(TrackedOperation const&) = delete;
        TrackedOperation& operator=(TrackedOperation const&) = delete;

    private:
        uint32 _slot;
    };

    /**
     * @brief Starts the watchdog that logs a stall.
     *
     * A dedicated thread polls the tracked operations. When one has been running for
     * longer than @p timeout, the stacks of all tracked threads are written to stderr
     * and to a log file next to the crash reports. The process keeps running.
     */
    YC_COMMON_API void StartDeadlockDetector(Milliseconds timeout);
}

#endif // _YCORE_DEADLOCK_DETECTOR_H_
