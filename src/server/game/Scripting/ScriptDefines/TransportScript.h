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

#ifndef SCRIPT_OBJECT_TRANSPORT_SCRIPT_H_
#define SCRIPT_OBJECT_TRANSPORT_SCRIPT_H_

#include "ScriptObject.h"

class Map;

class TransportScript : public ScriptObject, public UpdatableScript<Transport>
{
protected:
    TransportScript(const char* name);

public:
    [[nodiscard]] bool IsDatabaseBound() const override { return true; }

    // Called when a passenger joins the transport: a player or creature that boards
    // it, or a static creature/gameobject loaded with the transport's grid.
    virtual void OnAddPassenger(Transport* /*transport*/, WorldObject* /*passenger*/) { }

    // Called when a passenger leaves the transport, including a static passenger
    // unloaded with the transport's grid.
    virtual void OnRemovePassenger(Transport* /*transport*/, WorldObject* /*passenger*/) { }

    // Called after the transport has been added to its map, when its map and
    // instance id are known. Like OnTransportDestroy, this is broadcast to every
    // registered TransportScript, not only to the one bound to the transport.
    virtual void OnTransportCreate(Transport* /*transport*/) { }

    // Called every time the transport is added to a map, including when a motion
    // transport moves to another map. OnTransportCreate only announces the first
    // one, so this is the hook a per-map observer uses. Paired with
    // OnTransportLeaveMap.
    virtual void OnTransportEnterMap(Transport* /*transport*/, Map* /*map*/) { }

    // Called every time the transport is removed from a map, including the removal
    // that precedes a move to another map and the one that precedes destruction.
    virtual void OnTransportLeaveMap(Transport* /*transport*/, Map* /*map*/) { }

    // Called every transport update tick. Unlike OnUpdate, this is not gated on
    // the transport's script id, so it reaches scripts that are not bound to it.
    virtual void OnTransportUpdate(Transport* /*transport*/, uint32 /*diff*/) { }

    // Called when a transport is destroyed, before it is removed from the world.
    virtual void OnTransportDestroy(Transport* /*transport*/) { }

    // Called for each waypoint the transport advances to, not for every position
    // update.
    virtual void OnRelocate(Transport* /*transport*/, uint32 /*waypointId*/, uint32 /*mapId*/, float /*x*/, float /*y*/, float /*z*/) { }
};

#endif
