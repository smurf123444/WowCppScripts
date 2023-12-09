
#include "SpellAuraEffects.h"
#include "SpellMgr.h"
#include "SpellScript.h"

#include "SpellInfo.h"
#include "TemporarySummon.h"
enum CustomSpells
{
    //Primary Spells
    MORTAL_STRIKE                   = 51690,
    MORTAL_STRIKE_CRIT              = 51690,
    //Secondary spell
    SLAM_SPELL                      = 12345,
    //Auras
    REND_AURA                       = 12345,
    BLEED_AURA                      = 12345,

};

// 51690 - Killing Spree
#define CustomSpellScriptName "spell_custom_krein_test"
class spell_custom_krein_test : public SpellScriptLoader
{
public:
    spell_custom_krein_test() : SpellScriptLoader(CustomSpellScriptName) { }

    class spell_custom_krein_test_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_custom_krein_test_SpellScript);

        bool Validate(SpellInfo const* _spellProto) override
        {
            spellProto = _spellProto;
            return ValidateSpellInfo({ MORTAL_STRIKE });
        }

        SpellCastResult CheckCast()
        {
           // Kologarn area, MORTAL_STRIKE should not work
            if (GetCaster()->GetMapId() == 603  && GetCaster()->GetDistance2d(1766.936f, -24.748f) < 50.0f)
                return SPELL_FAILED_CANT_DO_THAT_RIGHT_NOW;
            return SPELL_CAST_OK; 
        }

        void FilterTargets(std::list<WorldObject*>& targets)
        {
            if (targets.empty() || GetCaster()->GetVehicleBase() || GetCaster()->HasUnitState(UNIT_STATE_ROOT))
                FinishCast(SPELL_FAILED_OUT_OF_RANGE);
            else
            {
                // Added attribute not breaking stealth, removes auras here
                GetCaster()->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_CAST);
                GetCaster()->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_SPELL_ATTACK);
            }

            }

        void HandleDummy(SpellEffIndex )
        {
            Unit* caster = GetCaster();
            uint32 spellId = RAND(MORTAL_STRIKE, MORTAL_STRIKE_CRIT);
            caster->CastSpell(caster, MORTAL_STRIKE, true, nullptr);
        }

        
        void HandleAfterCast()
        {
            Unit* caster = GetCaster();
            Player* target = GetHitPlayer();

            // Check if there is a valid target and caster
            if (!target || !caster)
                return;
            // Check if the target has the REND_AURA and if the SLAM_SPELL is on cooldown for the caster
            if (target->GetAura(REND_AURA) && caster->HasSpellCooldown(SLAM_SPELL)) {
                caster->ApplyCastTimePercentMod(0, true);
                int32 test = 1000;
                caster->ModSpellCastTime(spellProto, test, nullptr);
            }
        }

        void Register() override
        {
            AfterCast += SpellCastFn(spell_custom_krein_test_SpellScript::HandleAfterCast);
            OnCheckCast += SpellCheckCastFn(spell_custom_krein_test_SpellScript::CheckCast);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_custom_krein_test_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
            OnEffectHitTarget += SpellEffectFn(spell_custom_krein_test_SpellScript::HandleDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
            
        }

        private:
            std::list<WorldObject*> _targets;
            SpellInfo const* spellProto;
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_custom_krein_test_SpellScript();
    }

    class spell_custom_krein_test_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_custom_krein_test_AuraScript);

        bool Validate(SpellInfo const* ) override
        {
            return ValidateSpellInfo(
                {
                    REND_AURA
                });
        }

        void HandleApply(AuraEffect const*, AuraEffectHandleModes )
        {
            GetTarget()->CastSpell(GetTarget(), REND_AURA, true);
        }

        void HandleEffectPeriodic(AuraEffect const* aurEff)
        {
            
            Unit* caster = aurEff->GetCaster();

            if (Aura* aura = GetTarget()->GetAura(REND_AURA))
            {
                // If target has aura REnd then logic here.
                if (roll_chance_i(33))
                    caster->CastSpell(GetTarget(), MORTAL_STRIKE_CRIT, true);
                    caster->CastSpell(GetTarget(), BLEED_AURA, true);
            }
        }

        void HandleRemove(AuraEffect const* , AuraEffectHandleModes)
        {
            GetTarget()->RemoveAurasDueToSpell(REND_AURA);
        }

        void Register() override
        {
            AfterEffectApply += AuraEffectApplyFn(spell_custom_krein_test_AuraScript::HandleApply, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_custom_krein_test_AuraScript::HandleEffectPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            AfterEffectRemove += AuraEffectRemoveFn(spell_custom_krein_test_AuraScript::HandleRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }

    public:
        void AddTarget(Unit* target)
        {
            _targets.push_back(target->GetGUID());
        }

    private:
        GuidList _targets;
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_custom_krein_test_AuraScript();
    }
};


void AddSC_custom_spell_scripts()
{
     new spell_custom_krein_test();
}
