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

#ifndef _GROUPMGR_H
#define _GROUPMGR_H

#include "Group.h"

#include <functional>
#include <mutex>
#include <utility>
#include <vector>

// Holds GetSocialLock() and queues work that must run after it is released:
// sends and script hooks dispatch into code that takes locks of its own.
class GroupSocialLock
{
public:
    GroupSocialLock();
    ~GroupSocialLock();

    GroupSocialLock(GroupSocialLock const&) = delete;
    GroupSocialLock& operator=(GroupSocialLock const&) = delete;

    static bool IsHeld();
    static void Defer(std::function<void()>&& work);
    static void Send(ObjectGuid const& receiver, WorldPacket const& packet);

    // Runs the callback with the social lock released on this thread, so a
    // synchronous hook cannot invert the lock order. The guard stays held.
    template <typename Callback>
    static void InvokeUnlocked(Callback&& callback)
    {
        uint32 held = ReleaseForCallback();
        if (!held)
        {
            callback();
            return;
        }

        try
        {
            callback();
        }
        catch (...)
        {
            ReacquireAfterCallback(held);
            throw;
        }

        ReacquireAfterCallback(held);
    }

private:
    static uint32 ReleaseForCallback();
    static void ReacquireAfterCallback(uint32 held);

    std::unique_lock<std::recursive_mutex> _lock;
};

class GroupMgr
{
private:
    GroupMgr();
    ~GroupMgr();

    friend class GroupSocialLock;

    // Lock order, outer -> inner: world/session > social (this) > guild/channel/socialmgr > map > leaf stores.
    // Snapshot-then-act; never hold across I/O.
    static std::recursive_mutex* GetSocialLock()
    {
        static std::recursive_mutex lock;
        return &lock;
    }

public:
    static GroupMgr* instance();

    typedef std::map<uint32, Group*> GroupContainer;

    Group* GetGroupByGUID(ObjectGuid::LowType guid) const;

    void InitGroupIds();
    void RegisterGroupId(ObjectGuid::LowType groupId);
    ObjectGuid::LowType GenerateGroupId();

    void LoadGroups();
    void AddGroup(Group* group);
    void RemoveGroup(Group* group);
    // Crash-proof: only the manager frees groups. Destroy unregisters, marks
    // the group disbanded (zombie: live memory, all ops no-op) and defers the
    // actual delete so stale holders cannot UAF. Never `delete` a Group outside.
    void DestroyGroup(Group* group);

protected:
    typedef std::vector<bool> GroupIds;
    GroupIds            _groupIds;
    ObjectGuid::LowType _nextGroupId;
    GroupContainer      GroupStore;
    // Zombie graveyard: (group, timestamp). Swept after a grace period.
    std::vector<std::pair<Group*, uint32>> _groupGraveyard;
    void SweepGroupGraveyard();
};

#define sGroupMgr GroupMgr::instance()

#endif
