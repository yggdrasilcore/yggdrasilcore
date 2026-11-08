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

#ifndef SCRIPT_OBJECT_ALL_CREATURE_SCRIPT_H_
#define SCRIPT_OBJECT_ALL_CREATURE_SCRIPT_H_

#include "ScriptObject.h"

class AllCreatureScript : public ScriptObject
{
protected:
    AllCreatureScript(const char* name);

public:
    // Called from End of Creature Update.
    virtual void OnAllCreatureUpdate(Creature* /*creature*/, uint32 /*diff*/) { }

    // Called just before the level of the creature is set.
    virtual void OnBeforeCreatureSelectLevel(const CreatureTemplate* /*cinfo*/, Creature* /*creature*/, uint8& /*level*/) { }

    // Called from End of Creature SelectLevel.
    virtual void OnCreatureSelectLevel(const CreatureTemplate* /*cinfo*/, Creature* /*creature*/) { }

    /**
     * @brief This hook runs after add creature in world
     *
     * @param creature Contains information about the Creature
     */
    virtual void OnCreatureAddWorld(Creature* /*creature*/) { }

    /**
     * @brief This hook runs after remove creature in world
     *
     * @param creature Contains information about the Creature
     */
    virtual void OnCreatureRemoveWorld(Creature* /*creature*/) { }

    /**
     * @brief This hook runs after creature has been saved to DB
     *
     * @param creature Contains information about the Creature
    */
    virtual void OnCreatureSaveToDB(Creature* /*creature*/) { }

    /**
     * @brief This hook runs when a creature spawns or respawns.
     */
    virtual void OnCreatureSpawn(Creature* /*creature*/) { }

    /**
     * @brief This hook runs when a creature reaches a movement waypoint.
     */
    virtual void OnCreatureMovementInform(Creature* /*creature*/, uint32 /*type*/, uint32 /*id*/) { }

    /**
     * @brief This hook runs when a creature takes damage. The damage can be modified.
     */
    virtual void OnCreatureDamageTaken(Creature* /*creature*/, Unit* /*attacker*/, uint32& /*damage*/) { }

    /**
     * @brief This hook runs when a creature reaches its home position after evading.
     */
    virtual void OnCreatureReachedHome(Creature* /*creature*/) { }

    /**
     * @brief This hook runs when a creature has a unit in its line of sight.
     */
    virtual void OnCreatureMoveInLineOfSight(Creature* /*creature*/, Unit* /*who*/) { }

    /**
     * @brief This hook runs when a creature kills a unit.
     */
    virtual void OnCreatureKilledUnit(Creature* /*creature*/, Unit* /*victim*/) { }

    /**
     * @brief This hook runs before a creature starts attacking a new target.
     */
    virtual void OnCreaturePreCombat(Creature* /*creature*/, Unit* /*target*/) { }

    /**
     * @brief This hook runs when a creature is hit by a spell.
     */
    virtual void OnCreatureHitBySpell(Creature* /*creature*/, WorldObject* /*caster*/, SpellInfo const* /*spell*/) { }

    /**
     * @brief This hook runs when a creature's spell hits a target.
     */
    virtual void OnCreatureSpellHitTarget(Creature* /*creature*/, WorldObject* /*target*/, SpellInfo const* /*spell*/) { }

    /**
     * @brief This hook runs when a creature is summoned.
     */
    virtual void OnCreatureSummoned(Creature* /*creature*/, WorldObject* /*summoner*/) { }

    /**
     * @brief This hook runs when a creature summons another creature.
     */
    virtual void OnCreatureJustSummoned(Creature* /*creature*/, Creature* /*summon*/) { }

    /**
     * @brief This hook runs when a summoned creature despawns.
     */
    virtual void OnCreatureSummonedDespawn(Creature* /*creature*/, Creature* /*summon*/) { }

    /**
     * @brief This hook runs when a summoned creature dies.
     */
    virtual void OnCreatureSummonedDies(Creature* /*creature*/, Creature* /*summon*/, Unit* /*killer*/) { }

    /**
     * @brief This hook runs when a creature receives an emote.
     */
    virtual void OnCreatureReceiveEmote(Creature* /*creature*/, Player* /*player*/, uint32 /*emoteId*/) { }

    /**
     * @brief This hook runs when a creature's corpse is removed. The respawn delay can be modified.
     */
    virtual void OnCreatureCorpseRemoved(Creature* /*creature*/, uint32& /*respawnDelay*/) { }

    /**
     * @brief This hook runs when a creature's owner is attacked.
     */
    virtual void OnCreatureOwnerAttackedBy(Creature* /*creature*/, Unit* /*attacker*/) { }

    /**
     * @brief This hook runs when a creature's owner attacks a target.
     */
    virtual void OnCreatureOwnerAttacked(Creature* /*creature*/, Unit* /*target*/) { }

    /**
     * @brief This hook called when a player opens a gossip dialog with the creature.
     *
     * @param player Contains information about the Player
     * @param creature Contains information about the Creature
     *
     * @return False if you want to continue, true if you want to disable
     */
    [[nodiscard]] virtual bool CanCreatureGossipHello(Player* /*player*/, Creature* /*creature*/) { return false; }

    /**
     * @brief This hook called when a player selects a gossip item in the creature's gossip menu.
     *
     * @param player Contains information about the Player
     * @param creature Contains information about the Creature
     * @param sender Contains information about the sender type
     * @param action Contains information about the action id
     *
     * @return False if you want to continue, true if you want to disable
     */
    [[nodiscard]] virtual bool CanCreatureGossipSelect(Player* /*player*/, Creature* /*creature*/, uint32 /*sender*/, uint32 /*action*/) { return false; }

    /**
     * @brief This hook called when a player selects a gossip with a code in the creature's gossip menu.
     *
     * @param player Contains information about the Player
     * @param creature Contains information about the Creature
     * @param sender Contains information about the sender type
     * @param action Contains information about the action id
     * @param code Contains information about the code entered
     *
     * @return True if you want to continue, false if you want to disable
     */
    [[nodiscard]] virtual bool CanCreatureGossipSelectCode(Player* /*player*/, Creature* /*creature*/, uint32 /*sender*/, uint32 /*action*/, const char* /*code*/) { return false; }

    // Called when a player accepts a quest from the creature.
    [[nodiscard]] virtual bool CanCreatureQuestAccept(Player* /*player*/, Creature* /*creature*/, Quest const* /*quest*/) { return false; }

    // Called when a player selects a quest reward.
    [[nodiscard]] virtual bool CanCreatureQuestReward(Player* /*player*/, Creature* /*creature*/, Quest const* /*quest*/, uint32 /*opt*/) { return false; }

    // Called when a CreatureAI object is needed for the creature.
    [[nodiscard]] virtual CreatureAI* GetCreatureAI(Creature* /*creature*/) const { return nullptr; }

    //Called Whenever the UNIT_BYTE2_FLAG_FFA_PVP Bit is set on the creature
    virtual void OnFfaPvpStateUpdate(Creature* /*creature*/, bool /*InPvp*/) {}
};

#endif
