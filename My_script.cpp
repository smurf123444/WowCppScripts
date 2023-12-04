#include "GameTime.h"
#include "Player.h"
#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "SpellScript.h"
#include "TaskScheduler.h"

enum Say
{
    SAY_TELEPORT = 0,
    SAY_AGGRO,
    SAY_KILL,
};

enum Spells
{
/*     SPELL_MARK_OF_FROST       = 23182,
    SPELL_MARK_OF_FROST_AURA  = 23184,
    SPELL_AURA_OF_FROST       = 23186,
    SPELL_MANA_STORM          = 21097,
    SPELL_CHILL               = 21098,
    SPELL_FROST_BREATH        = 21099,
    SPELL_REFLECT             = 22067,
    SPELL_CLEAVE              = 19983,
    SPELL_ARCANE_VACUUM       = 21147,
    SPELL_ARCANE_VACUUM_TP    = 21150 */
    SPELL_STORMSTRIKE         = 17364,
    SPELL_CHAIN_LIGHTNING     = 49271,
    SPELL_SHOCKWAVE           = 75417,
    SPELL_GRIP_OF_AGONY       = 70572, // Intro
};


Position const chokePos[6] =
{
    {-514.4834f, 2211.334f, 549.2887f, 0.0f}, // High Overlord Saurfang/Muradin Bronzebeard
    {-510.1081f, 2211.592f, 546.3773f, 0.0f}, // front left
    {-513.3210f, 2211.396f, 551.2882f, 0.0f}, // front right
    {-507.3684f, 2210.353f, 545.7497f, 0.0f}, // back middle
    {-507.0486f, 2212.999f, 545.5512f, 0.0f}, // back left
    {-510.7041f, 2211.069f, 546.5298f, 0.0f}  // back right
};


class boss_azuregos : public CreatureScript
{
public:

    boss_azuregos() : CreatureScript("my_custom_boss") { }

    struct boss_azuregosAI : public ScriptedAI
    {
        boss_azuregosAI(Creature* creature) : ScriptedAI(creature)
        {
            scheduler.SetValidator([this]
            {
                return !me->HasUnitState(UNIT_STATE_CASTING);
            });
        }

        void Reset() override
        {
            scheduler.CancelAll();
            me->SetNpcFlag(UNIT_NPC_FLAG_GOSSIP);
            me->RestoreFaction();
/*             me->GetMap()->DoForAllPlayers([&](Player* p)
                {
                    if (p->GetZoneId() == me->GetZoneId())
                    {
                        p->RemoveAurasDueToSpell(SPELL_CHILL);
                        p->RemoveAurasDueToSpell(SPELL_FROST_BREATH);
                    }
                }); */
        }

        void KilledUnit(Unit* victim) override
        {
            if (victim && victim->GetTypeId() == TYPEID_PLAYER)
            {
                Talk(SAY_KILL);
               // victim->CastSpell(victim, SPELL_MARK_OF_FROST, true);
            }
        }

/*         void JustEngagedWith(Unit* who) override
        {

            _isBelow20Pct = false;
            _isThirdPhase = false;
            _bombCount = 0;
            _mysticBuffetStack = 0;

           // summons.DespawnAll();
            events.Reset();
            events.ScheduleEvent(EVENT_BERSERK, 10min);
            events.ScheduleEvent(EVENT_AIR_PHASE, 50s);
            events.ScheduleEvent(EVENT_CLEAVE, 10s, EVENT_GROUP_LAND_PHASE);
            events.ScheduleEvent(EVENT_TAIL_SMASH, 20s, EVENT_GROUP_LAND_PHASE);
            events.ScheduleEvent(EVENT_FROST_BREATH, 8s, 12s, EVENT_GROUP_LAND_PHASE);
            events.ScheduleEvent(EVENT_UNCHAINED_MAGIC, 9s, 14s, EVENT_GROUP_LAND_PHASE);
            events.ScheduleEvent(EVENT_ICY_GRIP, 33s + 500ms, EVENT_GROUP_LAND_PHASE);

            me->setActive(true);
            me->SetInCombatWithZone();
           // instance->SetBossState(DATA_SINDRAGOSA, IN_PROGRESS);

            me->CastSpell(me, SPELL_FROST_AURA, true);
            me->CastSpell(me, SPELL_PERMAEATING_CHILL, true);
            Talk(SAY_AGGRO);
        } */

        void JustEngagedWith(Unit*) override
        {
            //DoCastSelf(SPELL_MARK_OF_FROST_AURA);
            Talk(SAY_AGGRO);

/*             scheduler
                .Schedule(7s, [this](TaskContext context)
                {
                    DoCastVictim(SPELL_STORMSTRIKE);
                    context.Repeat(7s);
                })
                .Schedule(5s, 17s, [this](TaskContext context)
                {
                    DoCastRandomTarget(SPELL_CHAIN_LIGHTNING);
                    context.Repeat(7s, 13s);
                })
                .Schedule(10s, 30s, [this](TaskContext context)
                {
                    DoCastVictim(SPELL_SHOCKWAVE);
                    context.Repeat(13s, 25s);
                }); */
/*                 .Schedule(2s, 8s, [this](TaskContext context)
                {
                    DoCastVictim(SPELL_FROST_BREATH);
                    context.Repeat(10s, 15s);
                })
                .Schedule(30s, [this](TaskContext context)
                {
                    Talk(SAY_TELEPORT);
                    DoCastAOE(SPELL_ARCANE_VACUUM);
                    context.Repeat(30s);
                })
                .Schedule(15s, 30s, [this](TaskContext context)
                {
                    DoCastSelf(SPELL_REFLECT);
                    context.Repeat(20s, 35s);
                });  */
        } 
        void SpellHitTarget(Unit* target, SpellInfo const* spell) override
        {
            switch (spell->Id)
            {
                case SPELL_SHOCKWAVE:
                     DoResetThreat(target);
                    // Resets the specified unit's threat list (me if not specified) - does not delete entries, just sets their threat to zero
                    DoResetThreatList();
                    DoStartMovement(target, 10.0f, 3.0f);
                    break;
            }
        }
        void SpellHit(Unit* caster, SpellInfo const* spell) override
        {
            if (spell->Id == SPELL_SHOCKWAVE)
            {
            
                DoResetThreat(caster);
                // Resets the specified unit's threat list (me if not specified) - does not delete entries, just sets their threat to zero
                DoResetThreatList();
                DoStartMovement(caster, 10.0f, 3.0f);
            }
        }

        void JustDied(Unit* /*killer*/) override
        {
/*             me->RemoveAurasDueToSpell(SPELL_MARK_OF_FROST);
            me->GetMap()->DoForAllPlayers([&](Player* p)
                {
                    if (p->GetZoneId() == me->GetZoneId())
                    {

                        p->RemoveAurasDueToSpell(SPELL_MARK_OF_FROST);
                        p->RemoveAurasDueToSpell(SPELL_AURA_OF_FROST);
                        p->RemoveAurasDueToSpell(SPELL_CHILL);
                        p->RemoveAurasDueToSpell(SPELL_FROST_BREATH);
                    }
                }); */

            me->SetRespawnTime(urand(2 * DAY, 3 * DAY));
            me->SaveRespawnTime();
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
            {
                return;
            }

            scheduler.Update(diff, [this]
            {
                DoMeleeAttackIfReady();
            });
        }

            private:
        uint8 _bombCount;
        uint8 _mysticBuffetStack;
        bool _isBelow20Pct;
        bool _isThirdPhase;
        bool _isInAirPhase;
        bool _isLanding;
    };

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 /*action*/) override
    {
        CloseGossipMenuFor(player);
        creature->SetFaction(FACTION_ENEMY);
        creature->AI()->AttackStart(player);
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_azuregosAI(creature);
    }
};
/* 
 Arcane Vacuum: 21147
class spell_arcane_vacuum : public SpellScript
{
    PrepareSpellScript(spell_arcane_vacuum);

    bool Validate(SpellInfo const* ) override
    {
        return ValidateSpellInfo({ SPELL_ARCANE_VACUUM_TP });
    }

    void HandleOnHit()
    {
        Unit* caster = GetCaster();
        Unit* hitUnit = GetHitUnit();
        if (caster && hitUnit)
        {
            caster->GetThreatMgr().ModifyThreatByPercent(hitUnit, -100);
            caster->CastSpell(hitUnit, SPELL_ARCANE_VACUUM_TP, true);
        }
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_arcane_vacuum::HandleOnHit);
    }
};

// Mark of Frost - Triggered Spell
class spell_mark_of_frost_freeze : public SpellScript
{
    PrepareSpellScript(spell_mark_of_frost_freeze);

    bool Validate(SpellInfo const* ) override
    {
        return ValidateSpellInfo({ SPELL_MARK_OF_FROST, SPELL_AURA_OF_FROST });
    }

    void HandleOnHit()
    {
        Unit* caster = GetCaster();
        Unit* hitUnit = GetHitUnit();
        if (caster && hitUnit && hitUnit->HasAura(SPELL_MARK_OF_FROST) && !hitUnit->HasAura(SPELL_AURA_OF_FROST))
        {
            hitUnit->CastSpell(hitUnit, SPELL_AURA_OF_FROST, true);
        }
    }

    void Register() override
    {
        OnHit += SpellHitFn(spell_mark_of_frost_freeze::HandleOnHit);
    }
}; */

void AddSCmy_script_class()
{
    new boss_azuregos();
/*     RegisterSpellScript(spell_arcane_vacuum);
    RegisterSpellScript(spell_mark_of_frost_freeze); */
}
