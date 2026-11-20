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

#include "TransportScript.h"
#include "ScriptMgr.h"
#include "ScriptMgrMacros.h"
#include "Transport.h"

void ScriptMgr::OnAddPassenger(Transport* transport, WorldObject* passenger)
{
    ASSERT(transport);
    ASSERT(passenger);

    ExecuteScript<TransportScript>([=](TransportScript* script)
    {
        script->OnAddPassenger(transport, passenger);
    });
}

void ScriptMgr::OnRemovePassenger(Transport* transport, WorldObject* passenger)
{
    ASSERT(transport);
    ASSERT(passenger);

    ExecuteScript<TransportScript>([=](TransportScript* script)
    {
        script->OnRemovePassenger(transport, passenger);
    });
}

void ScriptMgr::OnTransportCreate(Transport* transport)
{
    ASSERT(transport);

    ExecuteScript<TransportScript>([=](TransportScript* script)
    {
        script->OnTransportCreate(transport);
    });
}

void ScriptMgr::OnTransportUpdate(Transport* transport, uint32 diff)
{
    ASSERT(transport);

    if (auto tempScript = ScriptRegistry<TransportScript>::GetScriptById(transport->GetScriptId()))
    {
        tempScript->OnUpdate(transport, diff);
    }

    // Generic broadcast: every registered TransportScript sees every transport's tick.
    ExecuteScript<TransportScript>([=](TransportScript* script)
    {
        script->OnTransportUpdate(transport, diff);
    });
}

void ScriptMgr::OnTransportDestroy(Transport* transport)
{
    ASSERT(transport);

    ExecuteScript<TransportScript>([=](TransportScript* script)
    {
        script->OnTransportDestroy(transport);
    });
}

void ScriptMgr::OnRelocate(Transport* transport, uint32 waypointId, uint32 mapId, float x, float y, float z)
{
    ASSERT(transport);

    ExecuteScript<TransportScript>([=](TransportScript* script)
    {
        script->OnRelocate(transport, waypointId, mapId, x, y, z);
    });
}

TransportScript::TransportScript(const char* name)
    : ScriptObject(name)
{
    ScriptRegistry<TransportScript>::AddScript(this);
}

template class YC_GAME_API ScriptRegistry<TransportScript>;
