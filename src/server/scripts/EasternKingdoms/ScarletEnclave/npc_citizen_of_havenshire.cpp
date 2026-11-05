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
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "CreatureScript.h"
#include "MovementGenerator.h"
#include "ObjectGuid.h"
#include "ScriptedCreature.h"
#include "SpellInfo.h"

enum CitizenOfHavenshire
{
    NPC_CITIZEN_OF_HAVENS_HIRE_1 = 28576,
    NPC_CITIZEN_OF_HAVENS_HIRE_2 = 28577,
    NPC_THE_LICH_KING           = 28765,
    NPC_LICH_KING_SPAWN_GUID    = 128581,

    SPELL_RAIN_OF_DARKNESS      = 52149,
    SPELL_CORNERED_AND_ENRAGED  = 52262,
    SPELL_LICH_KING_VO_BLOCKER  = 58207,
    SPELL_DEATH_GRIP            = 49560,

    SOUND_COWER_1              = 14561,
    SOUND_COWER_2              = 14564,

    SAY_ENRAGED                 = 0,
    SAY_FEAR                    = 1,
    SAY_DEATH                   = 2,

    EVENT_RAIN_OF_DARKNESS      = 1,
    EVENT_COWER                 = 2
};

struct npc_citizen_of_havenshire : public ScriptedAI
{
    npc_citizen_of_havenshire(Creature* creature) : ScriptedAI(creature), _enraged(false), _waypointsPaused(false) { }

    void Reset() override
    {
        _enraged = false;
        _waypointsPaused = false;
        me->SetReactState(REACT_PASSIVE);
        me->setActive(true);
        me->ClearEmoteState();

        events.Reset();
        events.ScheduleEvent(EVENT_RAIN_OF_DARKNESS, 5s, 20s);
    }

    void InitializeAI() override
    {
        ScriptedAI::InitializeAI();
        StartPath();
    }

    void JustRespawned() override
    {
        Reset();
        StartPath();
    }

    void PathEndReached(uint32 pathId) override
    {
        if (pathId == me->GetWaypointPath())
            me->DespawnOrUnsummon(2s);
    }

    void JustEngagedWith(Unit* who) override
    {
        PauseWaypoints();

        Unit* talkTarget = who && who->IsPlayer() ? who : me->SelectNearestPlayer(40.0f);

        // Rain of Darkness victims are completely passive and do not react to combat.
        if (me->HasAura(SPELL_RAIN_OF_DARKNESS))
        {
            _enraged = false;
            me->SetReactState(REACT_PASSIVE);
            return;
        }

        if (roll_chance_i(50))
        {
            _enraged = false;
            me->SetReactState(REACT_PASSIVE);
            me->HandleEmoteCommand(EMOTE_ONESHOT_EXCLAMATION);
            me->PlayDirectSound(me->GetEntry() == NPC_CITIZEN_OF_HAVENS_HIRE_1 ? SOUND_COWER_1 : SOUND_COWER_2);
            Talk(SAY_FEAR, talkTarget);
            events.ScheduleEvent(EVENT_COWER, 2s);
        }
        else
        {
            _enraged = true;
            me->SetReactState(REACT_AGGRESSIVE);
            DoCastSelf(SPELL_CORNERED_AND_ENRAGED, true);
            Talk(SAY_ENRAGED, talkTarget);
            AttackStart(who);
        }

        if (roll_chance_i(50))
        {
            if (Creature* lichKing = me->GetMap()->GetCreature(ObjectGuid::Create<HighGuid::Unit>(NPC_THE_LICH_KING, NPC_LICH_KING_SPAWN_GUID)))
                lichKing->CastSpell(who, SPELL_LICH_KING_VO_BLOCKER);
        }
    }

    void JustDied(Unit* killer) override
    {
        Talk(SAY_DEATH, killer && killer->IsPlayer() ? killer : me->SelectNearestPlayer(40.0f));
    }

    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        bool hasRainOfDarkness = me->HasAura(SPELL_RAIN_OF_DARKNESS);

        me->GetThreatMgr().ClearAllThreat();
        me->CombatStop(true);

        // Keep the Rain of Darkness aura and leave everything else untouched.
        if (hasRainOfDarkness)
            return;

        me->SetLootRecipient(nullptr);
        me->ResetPlayerDamageReq();
        me->ClearLeash();

        events.CancelEvent(EVENT_COWER);
        me->ClearEmoteState();
        me->RemoveAurasDueToSpell(SPELL_CORNERED_AND_ENRAGED);

        _enraged = false;
        me->SetReactState(REACT_PASSIVE);

        me->GetMotionMaster()->Clear(false);
        if (_waypointsPaused)
            ResumeWaypoints();
    }

    void SpellHit(Unit* /*caster*/, SpellInfo const* spell) override
    {
        if (spell->Id == SPELL_RAIN_OF_DARKNESS)
        {
            me->SetHealth(me->CountPctFromMaxHealth(10));
            me->SetRegeneratingHealth(false);
        }
        else if (spell->Id == SPELL_DEATH_GRIP && me->HasAura(SPELL_RAIN_OF_DARKNESS))
        {
            me->RemoveAurasDueToSpell(SPELL_RAIN_OF_DARKNESS);
            me->SetRegeneratingHealth(true);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_RAIN_OF_DARKNESS:
                    if (!me->IsInCombat() && !me->HasAura(SPELL_RAIN_OF_DARKNESS) && roll_chance_i(30))
                        DoCastSelf(SPELL_RAIN_OF_DARKNESS);

                    events.ScheduleEvent(EVENT_RAIN_OF_DARKNESS, 5s, 20s);
                    break;
                case EVENT_COWER:
                    me->SetEmoteState(EMOTE_STATE_COWER);
                    break;
                default:
                    break;
            }
        }

        bool hasRainOfDarkness = me->HasAura(SPELL_RAIN_OF_DARKNESS);
        me->SetRegeneratingHealth(!hasRainOfDarkness);

        if (!me->IsInCombat() && _waypointsPaused && !me->HasAura(SPELL_RAIN_OF_DARKNESS))
            ResumeWaypoints();

        if (!me->IsInCombat())
            return;

        if (hasRainOfDarkness || !_enraged)
        {
            me->SetReactState(REACT_PASSIVE);

            me->SelectVictim();
            return;
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }

private:
    void StartPath()
    {
        if (uint32 pathId = me->GetWaypointPath())
            me->GetMotionMaster()->MovePath(pathId, false);
    }

    void PauseWaypoints()
    {
        if (MovementGenerator* generator = me->GetMotionMaster()->GetMotionSlot(MOTION_SLOT_IDLE))
        {
            if (generator->GetMovementGeneratorType() == WAYPOINT_MOTION_TYPE)
            {
                generator->Pause(0);
                _waypointsPaused = true;
            }
        }
    }

    void ResumeWaypoints()
    {
        _waypointsPaused = false;

        if (MovementGenerator* generator = me->GetMotionMaster()->GetMotionSlot(MOTION_SLOT_IDLE))
        {
            if (generator->GetMovementGeneratorType() == WAYPOINT_MOTION_TYPE)
                generator->Resume(0);
            else
                StartPath();
        }
        else
            StartPath();
    }

    bool _enraged;
    bool _waypointsPaused;
};

void AddSC_the_scarlet_enclave_citizens()
{
    RegisterCreatureAI(npc_citizen_of_havenshire);
}
