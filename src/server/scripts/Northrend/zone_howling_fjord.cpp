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
#include "ScriptedEscortAI.h"
#include "ScriptedGossip.h"
#include "SmartAI.h"
#include "SpellInfo.h"
#include "SpellScript.h"

class npc_attracted_reef_bull : public CreatureScript
{
public:
    npc_attracted_reef_bull() : CreatureScript("npc_attracted_reef_bull") { }

    struct npc_attracted_reef_bullAI : public NullCreatureAI
    {
        npc_attracted_reef_bullAI(Creature* creature) : NullCreatureAI(creature)
        {
            me->SetDisableGravity(true);
            if (me->IsSummon())
                if (Unit* owner = me->ToTempSummon()->GetSummonerUnit())
                    me->GetMotionMaster()->MovePoint(0, *owner);
        }

        void MovementInform(uint32  /*type*/, uint32  /*id*/) override
        {
            if (Creature* cow = me->FindNearestCreature(24797, 5.0f, true))
            {
                me->CastSpell(me, 44460, true);
                me->DespawnOrUnsummon(10s);
                cow->CastSpell(cow, 44460, true);
                cow->DespawnOrUnsummon(10s);
                if (me->IsSummon())
                    if (Unit* owner = me->ToTempSummon()->GetSummonerUnit())
                        owner->CastSpell(owner, 44463, true);
            }
        }

        void SpellHit(Unit* caster, SpellInfo const* spellInfo) override
        {
            if (caster && spellInfo->Id == 44454)
                me->GetMotionMaster()->MovePoint(0, *caster);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_attracted_reef_bullAI(creature);
    }
};

// The cleansing
enum TurmoilTexts
{
    SAY_TURMOIL_0                = 0,
    SAY_TURMOIL_1                = 1,
    SAY_TURMOIL_HALF_HP          = 2,
    SAY_TURMOIL_DEATH            = 3,
};

class npc_your_inner_turmoil : public CreatureScript
{
public:
    npc_your_inner_turmoil() : CreatureScript("npc_your_inner_turmoil") { }

    struct npc_your_inner_turmoilAI : public ScriptedAI
    {
        npc_your_inner_turmoilAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 timer;
        short phase;
        bool health50;

        void Reset() override
        {
            timer = 0;
            phase = 0;
            health50 = false;
        }

        void UpdateAI(uint32 diff) override
        {
            if (timer >= 6000 && phase < 2)
            {
                phase++;
                setphase(phase);
                timer = 0;
            }

            timer += diff;

            DoMeleeAttackIfReady();
        }

        void DamageTaken(Unit*, uint32& /*damage*/, DamageEffectType  /*damagetype*/, SpellSchoolMask  /*damageSchoolMask*/) override
        {
            if (HealthBelowPct(50) && !health50)
            {
                if (TempSummon const* tempSummon = me->ToTempSummon())
                {
                    if (WorldObject* summoner = tempSummon->GetSummonerUnit())
                    {
                        Talk(SAY_TURMOIL_HALF_HP, summoner);
                    }
                }

                health50 = true;
            }
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (TempSummon const* tempSummon = me->ToTempSummon())
            {
                if (WorldObject* summoner = tempSummon->GetSummonerUnit())
                {
                    Talk(SAY_TURMOIL_DEATH, summoner);
                }
            }
        }

        void setphase(short newPhase)
        {
            Unit* summoner = me->ToTempSummon() ? me->ToTempSummon()->GetSummonerUnit() : nullptr;
            if (!summoner || !summoner->IsPlayer())
                return;

            switch (newPhase)
            {
                case 1:
                    Talk(SAY_TURMOIL_0, summoner->ToPlayer());
                    return;
                case 2:
                {
                    Talk(SAY_TURMOIL_1, summoner->ToPlayer());
                    me->SetLevel(summoner->GetLevel());
                    me->SetFaction(FACTION_MONSTER);
                    if (me->GetExactDist(summoner) < 50.0f)
                    {
                        me->UpdatePosition(summoner->GetPositionX(), summoner->GetPositionY(), summoner->GetPositionZ(), 0.0f, true);
                        summoner->CastSpell(me, 50218, true); // clone caster
                        AttackStart(summoner);
                    }
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_your_inner_turmoilAI(creature);
    }
};

/*######
## npc_apothecary_hanes
######*/
enum Entries
{
    NPC_APOTHECARY_HANES         = 23784,
    NPC_HANES_FIRE_TRIGGER       = 23968,
    QUEST_TRAIL_OF_FIRE          = 11241,
    SPELL_COSMETIC_LOW_POLY_FIRE = 56274,
    SPELL_HEALING_POTION         = 17534
};

class npc_apothecary_hanes : public CreatureScript
{
public:
    npc_apothecary_hanes() : CreatureScript("npc_apothecary_hanes") { }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_TRAIL_OF_FIRE)
        {
            creature->SetFaction(player->GetTeamId() == TEAM_ALLIANCE ? FACTION_ESCORTEE_A_PASSIVE : FACTION_ESCORTEE_H_PASSIVE);
            creature->SetWalk(true);
            CAST_AI(npc_escortAI, (creature->AI()))->Start(true, player->GetGUID());
        }
        return true;
    }

    struct npc_Apothecary_HanesAI : public npc_escortAI
    {
        npc_Apothecary_HanesAI(Creature* creature) : npc_escortAI(creature) { }
        uint32 PotTimer;

        void Reset() override
        {
            SetDespawnAtFar(false);
            PotTimer = 10000; //10 sec cooldown on potion
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (Player* player = GetPlayerForEscort())
                player->FailQuest(QUEST_TRAIL_OF_FIRE);
        }

        void UpdateEscortAI(uint32 diff) override
        {
            if (HealthBelowPct(75))
            {
                if (PotTimer <= diff)
                {
                    DoCast(me, SPELL_HEALING_POTION, true);
                    PotTimer = 10000;
                }
                else PotTimer -= diff;
            }
            if (GetAttack() && UpdateVictim())
                DoMeleeAttackIfReady();
        }

        void WaypointReached(uint32 waypointId) override
        {
            Player* player = GetPlayerForEscort();
            if (!player)
                return;

            switch (waypointId)
            {
                case 1:
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->SetWalk(false);
                    break;
                case 23:
                    player->GroupEventHappens(QUEST_TRAIL_OF_FIRE, me);
                    me->DespawnOrUnsummon();
                    break;
                case 5:
                    if (Unit* Trigger = me->FindNearestCreature(NPC_HANES_FIRE_TRIGGER, 10.0f))
                        Trigger->CastSpell(Trigger, SPELL_COSMETIC_LOW_POLY_FIRE, false);
                    me->SetWalk(true);
                    break;
                case 6:
                    if (Unit* Trigger = me->FindNearestCreature(NPC_HANES_FIRE_TRIGGER, 10.0f))
                        Trigger->CastSpell(Trigger, SPELL_COSMETIC_LOW_POLY_FIRE, false);
                    me->SetWalk(false);
                    break;
                case 8:
                    if (Unit* Trigger = me->FindNearestCreature(NPC_HANES_FIRE_TRIGGER, 10.0f))
                        Trigger->CastSpell(Trigger, SPELL_COSMETIC_LOW_POLY_FIRE, false);
                    me->SetWalk(true);
                    break;
                case 9:
                    if (Unit* Trigger = me->FindNearestCreature(NPC_HANES_FIRE_TRIGGER, 10.0f))
                        Trigger->CastSpell(Trigger, SPELL_COSMETIC_LOW_POLY_FIRE, false);
                    break;
                case 10:
                    me->SetWalk(false);
                    break;
                case 13:
                    me->SetWalk(true);
                    break;
                case 14:
                    if (Unit* Trigger = me->FindNearestCreature(NPC_HANES_FIRE_TRIGGER, 10.0f))
                        Trigger->CastSpell(Trigger, SPELL_COSMETIC_LOW_POLY_FIRE, false);
                    me->SetWalk(false);
                    break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_Apothecary_HanesAI(creature);
    }
};

/*######
## npc_plaguehound_tracker
######*/

class npc_plaguehound_tracker : public CreatureScript
{
public:
    npc_plaguehound_tracker() : CreatureScript("npc_plaguehound_tracker") { }

    struct npc_plaguehound_trackerAI : public npc_escortAI
    {
        npc_plaguehound_trackerAI(Creature* creature) : npc_escortAI(creature) { }

        void Reset() override
        {
            ObjectGuid summonerGUID;
            if (me->IsSummon())
                if (Unit* summoner = me->ToTempSummon()->GetSummonerUnit())
                    if (summoner->IsPlayer())
                        summonerGUID = summoner->GetGUID();

            if (!summonerGUID)
                return;

            me->SetWalk(true);
            Start(false, summonerGUID);
        }

        void WaypointReached(uint32 waypointId) override
        {
            if (waypointId != 26)
                return;

            me->DespawnOrUnsummon();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_plaguehound_trackerAI(creature);
    }
};

/*######
## npc_razael_and_lyana
######*/

enum Razael
{
    QUEST_REPORTS_FROM_THE_FIELD = 11221,
    NPC_RAZAEL = 23998,
    NPC_LYANA = 23778,
    GOSSIP_TEXTID_RAZAEL1 = 11562,
    GOSSIP_TEXTID_RAZAEL2 = 11564,
    GOSSIP_TEXTID_LYANA1 = 11586,
    GOSSIP_TEXTID_LYANA2 = 11588
};

class npc_razael_and_lyana : public CreatureScript
{
public:
    npc_razael_and_lyana() : CreatureScript("npc_razael_and_lyana") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (creature->IsQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->GetQuestStatus(QUEST_REPORTS_FROM_THE_FIELD) == QUEST_STATUS_INCOMPLETE)
            switch (creature->GetEntry())
            {
                case NPC_RAZAEL:
                    if (!player->GetReqKillOrCastCurrentCount(QUEST_REPORTS_FROM_THE_FIELD, NPC_RAZAEL))
                    {
                        AddGossipItemFor(player, 8870, 0, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
                        SendGossipMenuFor(player, GOSSIP_TEXTID_RAZAEL1, creature->GetGUID());
                        return true;
                    }
                    break;
                case NPC_LYANA:
                    if (!player->GetReqKillOrCastCurrentCount(QUEST_REPORTS_FROM_THE_FIELD, NPC_LYANA))
                    {
                        AddGossipItemFor(player, 8879, 0, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
                        SendGossipMenuFor(player, GOSSIP_TEXTID_LYANA1, creature->GetGUID());
                        return true;
                    }
                    break;
            }
        SendGossipMenuFor(player, player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        ClearGossipMenuFor(player);
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF + 1:
                SendGossipMenuFor(player, GOSSIP_TEXTID_RAZAEL2, creature->GetGUID());
                player->TalkedToCreature(NPC_RAZAEL, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 2:
                SendGossipMenuFor(player, GOSSIP_TEXTID_LYANA2, creature->GetGUID());
                player->TalkedToCreature(NPC_LYANA, creature->GetGUID());
                break;
        }
        return true;
    }
};

enum RodinLightningSpells
{
    SPELL_RODIN_LIGHTNING_START = 44787,
    SPELL_RODIN_LIGHTNING_END   = 44791,

    NPC_RODIN                   = 24876
};

struct npc_rodin_lightning_enabler : public ScriptedAI
{
    npc_rodin_lightning_enabler(Creature* creature) : ScriptedAI(creature) {}

    void Reset() override
    {
        _scheduler.Schedule(1s, [this](TaskContext context)
        {
            if (Creature* rodin = me->FindNearestCreature(NPC_RODIN, 10.0f))
                DoCast(rodin, urand(SPELL_RODIN_LIGHTNING_START, SPELL_RODIN_LIGHTNING_END));

            context.Repeat(2s, 8s);
        });
    }

    void UpdateAI(uint32 /*diff*/) override
    {
        _scheduler.Update();
    }

private:
    TaskScheduler _scheduler;
};

enum HawkHunting
{
    SPELL_HAWK_HUNTING_ITEM = 44408
};

// 44407 - Spell hawk Hunting
class spell_hawk_hunting : public SpellScript
{
    PrepareSpellScript(spell_hawk_hunting);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_HAWK_HUNTING_ITEM });
    }

    void HandleScriptEffect(SpellEffIndex /*effIndex*/)
    {
        if (!GetCaster())
            return;

        GetCaster()->CastSpell(GetCaster(), SPELL_HAWK_HUNTING_ITEM, true);
        GetHitUnit()->ToCreature()->DespawnOrUnsummon();
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_hawk_hunting::HandleScriptEffect, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

enum ShoveltuskStag
{
    NPC_SHOVELTUSK_STAG             = 23691,
    SHOVELTUSK_SPAR_SEARCH_DIST     = 15,
    SHOVELTUSK_SPAR_FACE_OFF_MS     = 1500,
    SHOVELTUSK_SPAR_DURATION_MS     = 12000
};

constexpr float SHOVELTUSK_SPAR_MIN_HEALTH_PCT      = 60.0f;
// Fractions of the melee range. Below the minimum the stags have pushed into
// each other (heads clipping), so they briefly back off to the ideal distance.
constexpr float SHOVELTUSK_SPAR_MIN_SEPARATION_PCT  = 0.60f;
constexpr float SHOVELTUSK_SPAR_IDEAL_SEPARATION_PCT = 0.85f;

/// Male Shoveltusk Stags periodically challenge each other over territory or mates.
/// They square off, then ram their antlers together. The melee swing is the stag's
/// headbutt animation. creature_sparring keeps the mock fight from ever killing them.
class npc_shoveltusk_stag : public SmartAI
{
public:
    npc_shoveltusk_stag(Creature* creature) : SmartAI(creature), _nextSparCheck(urand(30, 120) * IN_MILLISECONDS) { }

    void UpdateAI(uint32 diff) override
    {
        if (_sparring)
        {
            UpdateSparring(diff);
            return;
        }

        SmartAI::UpdateAI(diff);

        if (_nextSparCheck <= diff)
        {
            _nextSparCheck = urand(60, 180) * IN_MILLISECONDS;
            TryStartSparring();
        }
        else
            _nextSparCheck -= diff;
    }

    void JustEngagedWith(Unit* who) override
    {
        // The sparring opener also enters combat; don't run the regular aggro
        // behaviour (Shoveltusk Charge) for it.
        if (_sparring && who && who->GetGUID() == _partnerGuid)
            return;

        SmartAI::JustEngagedWith(who);
    }

    void DamageTaken(Unit* attacker, uint32& damage, DamageEffectType damageType, SpellSchoolMask damageSchoolMask) override
    {
        if (_sparring && (!attacker || attacker->GetGUID() != _partnerGuid))
        {
            StopSparring(false);

            if (attacker && me->IsValidAttackTarget(attacker))
                me->Attack(attacker, true);
        }

        SmartAI::DamageTaken(attacker, damage, damageType, damageSchoolMask);
    }

    void OnCalculateMeleeDamageReceived(uint32& damage, Unit* attacker) override
    {
        // A territorial display, not a fight to the death: the tusks only
        // inflict minor damage on a sparring partner. This hook runs before
        // the damage packet is sent, so the client sees the reduced value.
        if (_sparring && attacker && attacker->GetGUID() == _partnerGuid)
            damage = std::max<uint32>(1, damage / 4);
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        if (_sparring)
            StopSparring(false);

        SmartAI::EnterEvadeMode(why);
    }

    void JustDied(Unit* killer) override
    {
        if (_sparring)
            StopSparring(false);

        SmartAI::JustDied(killer);
    }

private:
    void TryStartSparring()
    {
        if (!me->IsAlive() || me->IsInCombat() || me->IsInEvadeMode() || me->IsCharmedOwnedByPlayerOrPlayer()
            || me->GetHealthPct() < SHOVELTUSK_SPAR_MIN_HEALTH_PCT)
            return;

        std::list<Creature*> stags;
        me->GetCreatureListWithEntryInGrid(stags, NPC_SHOVELTUSK_STAG, SHOVELTUSK_SPAR_SEARCH_DIST);

        Creature* partner = nullptr;
        float bestDistance = SHOVELTUSK_SPAR_SEARCH_DIST;

        for (Creature* stag : stags)
        {
            if (stag == me || !stag->IsAlive() || stag->IsInCombat() || stag->IsInEvadeMode()
                || stag->IsCharmedOwnedByPlayerOrPlayer() || stag->GetHealthPct() < SHOVELTUSK_SPAR_MIN_HEALTH_PCT)
                continue;

            if (!me->InSamePhase(stag) || !me->IsWithinLOSInMap(stag))
                continue;

            npc_shoveltusk_stag* stagAI = dynamic_cast<npc_shoveltusk_stag*>(stag->AI());
            if (!stagAI || stagAI->_sparring)
                continue;

            float distance = me->GetDistance(stag);
            if (distance < bestDistance)
            {
                bestDistance = distance;
                partner = stag;
            }
        }

        if (partner)
            StartSparring(partner);
    }

    void StartSparring(Creature* partner)
    {
        npc_shoveltusk_stag* partnerAI = dynamic_cast<npc_shoveltusk_stag*>(partner->AI());
        if (!partnerAI)
            return;

        _sparring = true;
        _partnerGuid = partner->GetGUID();
        _sparTimer = 0;
        _faceOffTimer = SHOVELTUSK_SPAR_FACE_OFF_MS;

        partnerAI->_sparring = true;
        partnerAI->_partnerGuid = me->GetGUID();
        partnerAI->_sparTimer = 0;
        partnerAI->_faceOffTimer = SHOVELTUSK_SPAR_FACE_OFF_MS;

        // Prevent the herd from joining the sparring match.
        me->SetNoCallAssistance(true);
        partner->SetNoCallAssistance(true);

        me->SetFacingToObject(partner);
        partner->SetFacingToObject(me);

        me->Attack(partner, true);
        partner->Attack(me, true);

        // Unit::Attack arms the periodic assistance timer, which Creature::Update
        // would otherwise use to re-enable CallAssistance() while we spar.
        me->SetAssistanceTimer(0);
        partner->SetAssistanceTimer(0);

        // Only the stag that started the display moves. If both chase each
        // other they run through one another and the models end up overlapping.
        _isMoving = true;
        partnerAI->_isMoving = false;

        float const desiredSeparation = me->GetMeleeRange(partner) * SHOVELTUSK_SPAR_IDEAL_SEPARATION_PCT;
        if (me->GetExactDist(partner) < desiredSeparation)
            me->GetMotionMaster()->MoveBackwards(partner, desiredSeparation);
        else
            me->GetMotionMaster()->MoveChase(partner);

        partner->GetMotionMaster()->MoveIdle();
    }

    void UpdateSparring(uint32 diff)
    {
        Creature* partner = ObjectAccessor::GetCreature(*me, _partnerGuid);
        if (!partner || !partner->IsAlive() || !me->IsAlive() || !me->IsWithinLOSInMap(partner)
            || me->GetDistance(partner) > SHOVELTUSK_SPAR_SEARCH_DIST * 2)
        {
            StopSparring(true);
            return;
        }

        // Something may have cleared the attack state (e.g. a spell interrupt).
        if (me->GetVictim() != partner)
            me->Attack(partner, true);

        if (_faceOffTimer > diff)
        {
            _faceOffTimer -= diff;
            return;
        }

        _faceOffTimer = 0;

        _sparTimer += diff;
        if (_sparTimer >= SHOVELTUSK_SPAR_DURATION_MS)
        {
            StopSparring(true);
            return;
        }

        // Wait for the opening charge (or the initial back-off step) to finish.
        if (!me->movespline->Finalized())
            return;

        if (!me->IsWithinMeleeRange(partner))
        {
            if (_isMoving && me->GetMotionMaster()->GetCurrentMovementGeneratorType() != CHASE_MOTION_TYPE)
                me->GetMotionMaster()->MoveChase(partner);

            return;
        }

        float const meleeRange = me->GetMeleeRange(partner);

        // Safety net in case the spawns put them inside each other: the moving
        // stag takes a short step back to the ideal headbutt distance while the
        // other one waits, so neither swings from inside the other's model.
        if (me->GetExactDist(partner) < meleeRange * SHOVELTUSK_SPAR_MIN_SEPARATION_PCT)
        {
            if (_isMoving)
                me->GetMotionMaster()->MoveBackwards(partner, meleeRange * SHOVELTUSK_SPAR_IDEAL_SEPARATION_PCT);

            return;
        }

        // Let the normal melee swing play the headbutt animation. Even when
        // creature_sparring caps the damage, the client still receives the
        // fake-damage hit and plays the attack animation.
        DoMeleeAttackIfReady();
    }

    void StopSparring(bool returnHome)
    {
        if (!_sparring)
            return;

        Creature* partner = ObjectAccessor::GetCreature(*me, _partnerGuid);

        _sparring = false;
        _isMoving = false;
        _partnerGuid.Clear();
        _sparTimer = 0;
        _faceOffTimer = 0;
        _nextSparCheck = urand(60, 180) * IN_MILLISECONDS;

        me->AttackStop();
        if (partner)
            me->GetThreatMgr().ClearThreat(partner);

        if (me->GetThreatMgr().isThreatListEmpty())
            me->CombatStop(true);

        if (me->IsAlive())
        {
            if (returnHome)
                me->GetMotionMaster()->MoveTargetedHome();
            else
            {
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MoveIdle();
            }
        }

        if (partner && partner->IsAlive())
        {
            if (npc_shoveltusk_stag* partnerAI = dynamic_cast<npc_shoveltusk_stag*>(partner->AI()))
                if (partnerAI->_sparring && partnerAI->_partnerGuid == me->GetGUID())
                    partnerAI->StopSparring(returnHome);
        }
    }

    ObjectGuid _partnerGuid;
    uint32 _nextSparCheck;
    uint32 _sparTimer = 0;
    uint32 _faceOffTimer = 0;
    bool _sparring = false;
    bool _isMoving = false;
};

void AddSC_howling_fjord()
{
    new npc_attracted_reef_bull();
    new npc_your_inner_turmoil();
    new npc_apothecary_hanes();
    new npc_plaguehound_tracker();
    new npc_razael_and_lyana();
    RegisterCreatureAI(npc_rodin_lightning_enabler);
    RegisterCreatureAI(npc_shoveltusk_stag);
    RegisterSpellScript(spell_hawk_hunting);
}
