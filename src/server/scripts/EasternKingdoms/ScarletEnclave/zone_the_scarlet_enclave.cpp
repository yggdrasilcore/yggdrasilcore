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

#include "CreatureScript.h"
#include "PassiveAI.h"
#include "Player.h"
#include "ScriptedCreature.h"

/*####
## npc_valkyr_battle_maiden
####*/

enum Spells_VBM
{
    SPELL_REVIVE                = 51918
};

enum Says_VBM
{
    WHISPER_REVIVE              = 0
};

class npc_valkyr_battle_maiden : public CreatureScript
{
public:
    npc_valkyr_battle_maiden() : CreatureScript("npc_valkyr_battle_maiden") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_valkyr_battle_maidenAI(creature);
    }

    struct npc_valkyr_battle_maidenAI : public PassiveAI
    {
        npc_valkyr_battle_maidenAI(Creature* creature) : PassiveAI(creature) { }

        uint32 FlyBackTimer;
        float x, y, z;
        uint32 phase;

        void Reset() override
        {
            me->setActive(true);
            me->SetVisible(false);
            me->SetUnitFlag(UNIT_FLAG_NON_ATTACKABLE);
            me->SetCanFly(true);
            FlyBackTimer = 500;
            phase = 0;

            me->GetPosition(x, y, z);
            z += 4.0f;
            x -= 3.5f;
            y -= 5.0f;
            me->GetMotionMaster()->Clear(false);
            me->SetPosition(x, y, z, 0.0f);
        }

        void UpdateAI(uint32 diff) override
        {
            if (FlyBackTimer <= diff)
            {
                Player* player = nullptr;
                if (me->IsSummon())
                {
                    if (Unit * summoner = me->ToTempSummon()->GetSummonerUnit())
                    {
                        player = summoner->ToPlayer();
                    }
                }

                if (!player)
                {
                    phase = 3;
                }

                switch (phase)
                {
                    case 0:
                        me->SetWalk(false);
                        me->HandleEmoteCommand(EMOTE_STATE_FLYGRABCLOSED);
                        FlyBackTimer = 500;
                        break;
                    case 1:
                        if (player)
                        {
                            player->GetClosePoint(x, y, z, me->GetObjectSize());
                        }
                        z += 2.5f;
                        x -= 2.0f;
                        y -= 1.5f;
                        me->GetMotionMaster()->MovePoint(0, x, y, z);
                        if (player)
                        {
                            me->SetTarget(player->GetGUID());
                        }
                        me->SetVisible(true);
                        FlyBackTimer = 4500;
                        break;
                    case 2:
                        if (player && !player->isResurrectRequested())
                        {
                            me->HandleEmoteCommand(EMOTE_ONESHOT_CUSTOM_SPELL_01);
                            DoCast(player, SPELL_REVIVE, true);
                            Talk(WHISPER_REVIVE, player);
                        }
                        FlyBackTimer = 5000;
                        break;
                    case 3:
                        me->SetVisible(false);
                        FlyBackTimer = 3000;
                        break;
                    case 4:
                        me->DisappearAndDie();
                        break;
                    default:
                        //Nothing To DO
                        break;
                }
                ++phase;
            }
            else FlyBackTimer -= diff;
        }
    };
};

enum ScarletPeasant
{
    NPC_SCARLET_PEASANT         = 28557,

    SPELL_STUN_SELF             = 51604,
    SOUND_PEASANT_COWER         = 14561,

    SAY_PEASANT_COWER           = 0,

    EVENT_CHECK_NEAR_PLAYER     = 1,
    EVENT_COWER_APPLY           = 2,
    EVENT_COWER_SAY             = 3,
    EVENT_COWER_FAILSAFE        = 4,

    PHASE_DEFENSIVE             = 1,
    PHASE_COWERING              = 2,
    PHASE_ATTACKING             = 3,

    NEAR_PLAYER_RANGE           = 5,
    COWER_FAILSAFE_RANGE        = 40
};

struct npc_scarlet_peasant : public ScriptedAI
{
    npc_scarlet_peasant(Creature* creature) : ScriptedAI(creature), _phase(PHASE_DEFENSIVE), _healthCowerTriggered(false) { }

    void Reset() override
    {
        _phase = PHASE_DEFENSIVE;
        _healthCowerTriggered = false;

        me->RemoveAurasDueToSpell(SPELL_STUN_SELF);
        me->SetReactState(REACT_DEFENSIVE);
        me->SetEmoteState(EMOTE_STATE_WORK_MINING);

        events.Reset();
        events.ScheduleEvent(EVENT_CHECK_NEAR_PLAYER, 1s);
    }

    void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType, SpellSchoolMask) override
    {
        if (!attacker || !damage || !me->IsAlive() || _phase == PHASE_COWERING)
            return;

        if (_phase == PHASE_DEFENSIVE && damage < me->GetHealth())
        {
            if (roll_chance_i(50))
                StartAttack(attacker);
            else
                StartCower(me->SelectNearestPlayer(30));
        }
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_CHECK_NEAR_PLAYER:
                    if (_phase == PHASE_DEFENSIVE && !me->IsInCombat())
                    {
                        if (Unit* player = me->SelectNearestPlayer(NEAR_PLAYER_RANGE))
                        {
                            if (roll_chance_i(50))
                                StartAttack(player);
                            else
                                StartCower(player);
                        }
                    }

                    if (_phase == PHASE_DEFENSIVE)
                        events.ScheduleEvent(EVENT_CHECK_NEAR_PLAYER, 2s);
                    break;
                case EVENT_COWER_APPLY:
                    if (_phase == PHASE_COWERING)
                    {
                        me->SetReactState(REACT_PASSIVE);
                        DoCastSelf(SPELL_STUN_SELF, true);
                        me->PlayDirectSound(SOUND_PEASANT_COWER);
                        me->SetEmoteState(EMOTE_STATE_COWER);
                    }
                    break;
                case EVENT_COWER_SAY:
                    if (_phase == PHASE_COWERING)
                        Talk(SAY_PEASANT_COWER);
                    break;
                case EVENT_COWER_FAILSAFE:
                    if (_phase == PHASE_COWERING)
                    {
                        if (!me->SelectNearestPlayer(COWER_FAILSAFE_RANGE))
                        {
                            StopCowering();
                            break;
                        }

                        events.ScheduleEvent(EVENT_COWER_FAILSAFE, 1s);
                    }
                    break;
                default:
                    break;
            }
        }

        if (_phase == PHASE_COWERING)
            return;

        if (me->IsInCombat() && !_healthCowerTriggered && me->GetHealthPct() <= 50)
        {
            _healthCowerTriggered = true;
            StartCower(me->SelectNearestPlayer(30));
            return;
        }

        if (_phase == PHASE_ATTACKING)
        {
            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    }

private:
    void StartAttack(Unit* target)
    {
        if (!target || !me->IsValidAttackTarget(target))
            return;

        _phase = PHASE_ATTACKING;
        me->SetReactState(REACT_AGGRESSIVE);
        AttackStart(target);
    }

    void StartCower(Unit* target)
    {
        _phase = PHASE_COWERING;

        events.CancelEvent(EVENT_CHECK_NEAR_PLAYER);
        me->SetReactState(REACT_PASSIVE);
        me->AttackStop();

        if (target)
            me->SetFacingToObject(target);

        events.ScheduleEvent(EVENT_COWER_APPLY, 500ms);
        events.ScheduleEvent(EVENT_COWER_SAY, 1500ms);
        events.ScheduleEvent(EVENT_COWER_FAILSAFE, 1s);
    }

    void StopCowering()
    {
        _phase = PHASE_DEFENSIVE;
        _healthCowerTriggered = false;

        events.CancelEvent(EVENT_COWER_APPLY);
        events.CancelEvent(EVENT_COWER_SAY);
        events.CancelEvent(EVENT_COWER_FAILSAFE);

        me->RemoveAurasDueToSpell(SPELL_STUN_SELF);
        me->ClearEmoteState();
        me->SetEmoteState(EMOTE_STATE_WORK_MINING);
        me->SetReactState(REACT_DEFENSIVE);

        if (me->IsInCombat())
            EnterEvadeMode();
        else
            events.ScheduleEvent(EVENT_CHECK_NEAR_PLAYER, 1s);
    }

    int32 _phase;
    bool _healthCowerTriggered;
};

void AddSC_the_scarlet_enclave()
{
    new npc_valkyr_battle_maiden();
    RegisterCreatureAI(npc_scarlet_peasant);
}
