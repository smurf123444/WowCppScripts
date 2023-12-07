

#include "ScriptMgr.h"
#include "SpellAuraEffects.h"
#include "SpellMgr.h"
#include "SpellScript.h"
enum CustomSpells
{
    BLADE_FLURRY_EXTRA_ATTACK       = 22482,
    CHEAT_DEATH_COOLDOWN            = 31231,
    CHEATING_DEATH                  = 45182,
    GLYPH_OF_PREPARATION            = 56819,
    KILLING_SPREE                   = 51690,
    KILLING_SPREE_TELEPORT          = 57840,
    KILLING_SPREE_WEAPON_DMG        = 57841,
    KILLING_SPREE_DMG_BUFF          = 61851,
    PREY_ON_THE_WEAK                = 58670,
    SHIV_TRIGGERED                  = 5940,
    TRICKS_OF_THE_TRADE_DMG_BOOST   = 57933,
    TRICKS_OF_THE_TRADE_PROC        = 59628,
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

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo({ KILLING_SPREE });
        }

        SpellCastResult CheckCast()
        {
            // Kologarn area, Killing Spree should not work
            if (GetCaster()->GetMapId() == 603 /*Ulduar*/ && GetCaster()->GetDistance2d(1766.936f, -24.748f) < 50.0f)
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

        void HandleDummy(SpellEffIndex /*effIndex*/)
        {
            if (Aura* aura = GetCaster()->GetAura(KILLING_SPREE))
            {
                if (spell_custom_krein_test_AuraScript* script = dynamic_cast<spell_custom_krein_test_AuraScript*>(aura->GetScriptByName(CustomSpellScriptName)))
                    script->AddTarget(GetHitUnit());
            }
        }

        void Register() override
        {
            OnCheckCast += SpellCheckCastFn(spell_custom_krein_test_SpellScript::CheckCast);
            OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_custom_krein_test_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENEMY);
            OnEffectHitTarget += SpellEffectFn(spell_custom_krein_test_SpellScript::HandleDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const override
    {
        return new spell_custom_krein_test_SpellScript();
    }

    class spell_custom_krein_test_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_custom_krein_test_AuraScript);

        bool Validate(SpellInfo const* /*spellInfo*/) override
        {
            return ValidateSpellInfo(
                {
                    KILLING_SPREE_TELEPORT,
                    KILLING_SPREE_WEAPON_DMG,
                    KILLING_SPREE_DMG_BUFF
                });
        }

        void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            GetTarget()->CastSpell(GetTarget(), KILLING_SPREE_DMG_BUFF, true);
        }

        void HandleEffectPeriodic(AuraEffect const* /*aurEff*/)
        {
            while (!_targets.empty())
            {
                ObjectGuid guid = Acore::Containers::SelectRandomContainerElement(_targets);
                if (Unit* target = ObjectAccessor::GetUnit(*GetTarget(), guid))
                {
                    // xinef: target may be no longer valid
                    if (!GetTarget()->IsValidAttackTarget(target) || target->HasStealthAura() || target->HasInvisibilityAura())
                    {
                        _targets.remove(guid);
                        continue;
                    }

                    GetTarget()->CastSpell(target, KILLING_SPREE_TELEPORT, true);

                    // xinef: ensure fast coordinates switch, dont wait for client to send opcode
                    WorldLocation const& dest = GetTarget()->ToPlayer()->GetTeleportDest();
                    GetTarget()->ToPlayer()->UpdatePosition(dest, true);

                    GetTarget()->CastSpell(target, KILLING_SPREE_WEAPON_DMG, TriggerCastFlags(TRIGGERED_FULL_MASK & ~TRIGGERED_DONT_REPORT_CAST_ERROR));
                    break;
                }
                else
                    _targets.remove(guid);
            }
        }

        void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            GetTarget()->RemoveAurasDueToSpell(KILLING_SPREE_DMG_BUFF);
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

