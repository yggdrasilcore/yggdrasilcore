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

#include "AllCreatureScript.h"
#include "ScriptMgr.h"
#include "ScriptMgrMacros.h"

void ScriptMgr::OnCreatureAddWorld(Creature* creature)
{
    ASSERT(creature);

    ExecuteScript<AllCreatureScript>([&](AllCreatureScript* script)
    {
        script->OnCreatureAddWorld(creature);
    });
}

void ScriptMgr::OnCreatureRemoveWorld(Creature* creature)
{
    ASSERT(creature);

    ExecuteScript<AllCreatureScript>([&](AllCreatureScript* script)
    {
        script->OnCreatureRemoveWorld(creature);
    });
}

void ScriptMgr::OnCreatureSaveToDB(Creature* creature)
{
    ASSERT(creature);

    ExecuteScript<AllCreatureScript>([&](AllCreatureScript* script)
    {
        script->OnCreatureSaveToDB(creature);
    });
}

void ScriptMgr::OnCreatureSpawn(Creature* creature)
{
    ASSERT(creature);

    ExecuteScript<AllCreatureScript>([&](AllCreatureScript* script)
    {
        script->OnCreatureSpawn(creature);
    });
}

void ScriptMgr::OnCreatureMovementInform(Creature* creature, uint32 type, uint32 id)
{
    ASSERT(creature);

    ExecuteScript<AllCreatureScript>([&](AllCreatureScript* script)
    {
        script->OnCreatureMovementInform(creature, type, id);
    });
}

void ScriptMgr::OnCreatureDamageTaken(Creature* creature, Unit* attacker, uint32& damage)
{
    ASSERT(creature);

    ExecuteScript<AllCreatureScript>([&](AllCreatureScript* script)
    {
        script->OnCreatureDamageTaken(creature, attacker, damage);
    });
}

void ScriptMgr::OnCreatureReachedHome(Creature* creature)
{
    ASSERT(creature);

    ExecuteScript<AllCreatureScript>([&](AllCreatureScript* script)
    {
        script->OnCreatureReachedHome(creature);
    });
}

void ScriptMgr::OnCreatureMoveInLineOfSight(Creature* creature, Unit* who)
{
    ASSERT(creature);

    ExecuteScript<AllCreatureScript>([&](AllCreatureScript* script)
    {
        script->OnCreatureMoveInLineOfSight(creature, who);
    });
}

void ScriptMgr::OnCreatureKilledUnit(Creature* creature, Unit* victim)
{
    ASSERT(creature);

    ExecuteScript<AllCreatureScript>([&](AllCreatureScript* script)
    {
        script->OnCreatureKilledUnit(creature, victim);
    });
}

void ScriptMgr::OnCreaturePreCombat(Creature* creature, Unit* target)
{
    ASSERT(creature);

    ExecuteScript<AllCreatureScript>([&](AllCreatureScript* script)
    {
        script->OnCreaturePreCombat(creature, target);
    });
}

void ScriptMgr::OnCreatureHitBySpell(Creature* creature, WorldObject* caster, SpellInfo const* spell)
{
    ASSERT(creature);

    ExecuteScript<AllCreatureScript>([&](AllCreatureScript* script)
    {
        script->OnCreatureHitBySpell(creature, caster, spell);
    });
}

void ScriptMgr::OnCreatureSpellHitTarget(Creature* creature, WorldObject* target, SpellInfo const* spell)
{
    ASSERT(creature);

    ExecuteScript<AllCreatureScript>([&](AllCreatureScript* script)
    {
        script->OnCreatureSpellHitTarget(creature, target, spell);
    });
}

void ScriptMgr::OnCreatureSummoned(Creature* creature, WorldObject* summoner)
{
    ASSERT(creature);

    ExecuteScript<AllCreatureScript>([&](AllCreatureScript* script)
    {
        script->OnCreatureSummoned(creature, summoner);
    });
}

void ScriptMgr::OnCreatureJustSummoned(Creature* creature, Creature* summon)
{
    ASSERT(creature);

    ExecuteScript<AllCreatureScript>([&](AllCreatureScript* script)
    {
        script->OnCreatureJustSummoned(creature, summon);
    });
}

void ScriptMgr::OnCreatureSummonedDespawn(Creature* creature, Creature* summon)
{
    ASSERT(creature);

    ExecuteScript<AllCreatureScript>([&](AllCreatureScript* script)
    {
        script->OnCreatureSummonedDespawn(creature, summon);
    });
}

void ScriptMgr::OnCreatureSummonedDies(Creature* creature, Creature* summon, Unit* killer)
{
    ASSERT(creature);

    ExecuteScript<AllCreatureScript>([&](AllCreatureScript* script)
    {
        script->OnCreatureSummonedDies(creature, summon, killer);
    });
}

void ScriptMgr::OnCreatureReceiveEmote(Creature* creature, Player* player, uint32 emoteId)
{
    ASSERT(creature);

    ExecuteScript<AllCreatureScript>([&](AllCreatureScript* script)
    {
        script->OnCreatureReceiveEmote(creature, player, emoteId);
    });
}

void ScriptMgr::OnCreatureCorpseRemoved(Creature* creature, uint32& respawnDelay)
{
    ASSERT(creature);

    ExecuteScript<AllCreatureScript>([&](AllCreatureScript* script)
    {
        script->OnCreatureCorpseRemoved(creature, respawnDelay);
    });
}

void ScriptMgr::OnCreatureOwnerAttackedBy(Creature* creature, Unit* attacker)
{
    ASSERT(creature);

    ExecuteScript<AllCreatureScript>([&](AllCreatureScript* script)
    {
        script->OnCreatureOwnerAttackedBy(creature, attacker);
    });
}

void ScriptMgr::OnCreatureOwnerAttacked(Creature* creature, Unit* target)
{
    ASSERT(creature);

    ExecuteScript<AllCreatureScript>([&](AllCreatureScript* script)
    {
        script->OnCreatureOwnerAttacked(creature, target);
    });
}

void ScriptMgr::OnBeforeCreatureSelectLevel(const CreatureTemplate* cinfo, Creature* creature, uint8& level)
{
    ExecuteScript<AllCreatureScript>([&](AllCreatureScript* script)
    {
        script->OnBeforeCreatureSelectLevel(cinfo, creature, level);
    });
}

void ScriptMgr::OnCreatureSelectLevel(const CreatureTemplate* cinfo, Creature* creature)
{
    ExecuteScript<AllCreatureScript>([&](AllCreatureScript* script)
    {
        script->OnCreatureSelectLevel(cinfo, creature);
    });
}

//bool ScriptMgr::CanCreatureSendListInventory(Player* player, Creature* creature, uint32 vendorEntry)
//{
//    auto ret = IsValidBoolScript<AllCreatureScript>([&](AllCreatureScript* script)
//    {
//        return !script->CanCreatureSendListInventory(player, creature, vendorEntry);
//    });
//
//    if (ret && *ret)
//        return false;
//
//    return true;
//}

AllCreatureScript::AllCreatureScript(const char* name) :
    ScriptObject(name)
{
    ScriptRegistry<AllCreatureScript>::AddScript(this);
}

template class YC_GAME_API ScriptRegistry<AllCreatureScript>;
