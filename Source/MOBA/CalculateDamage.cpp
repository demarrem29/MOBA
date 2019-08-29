// Fill out your copyright notice in the Description page of Project Settings.


#include "CalculateDamage.h"
#include "MOBAAttributeSet.h"
#include "AbilitySystemComponent.h"

struct DamageStatics
{
	// Target's Defense Stats
	DECLARE_ATTRIBUTE_CAPTUREDEF(Health);
	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalDamageReduction);
	DECLARE_ATTRIBUTE_CAPTUREDEF(EnvironmentalDamageReduction);
	DECLARE_ATTRIBUTE_CAPTUREDEF(FlatDamageReduction);
	// Attacker's Offense Stats
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);
	DECLARE_ATTRIBUTE_CAPTUREDEF(SpellPower);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalDamage);
	

	DamageStatics()
	{
		// Capture the Target's Health attribute. Do not snapshot it, because we want to use the health value at the moment we apply the execution.
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, Health, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, PhysicalDamageReduction, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, EnvironmentalDamageReduction, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, FlatDamageReduction, Target, false);

		// Capture the Source's Damage Stats. We do want to snapshot this at the moment we create the GameplayEffectSpec that will execute the damage.
		// (imagine we fire a projectile: we create the GE Spec when the projectile is fired. When it hits the target, we want to use the AttackPower at the moment
		// the projectile was launched, not when it hits).
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, AttackPower, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, SpellPower, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, CriticalChance, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, CriticalDamage, Source, true); 
	}
};

DamageStatics& Damage()
{
	static DamageStatics It;
	return It;
}

UCalculateDamage::UCalculateDamage(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	RelevantAttributesToCapture.Add(Damage().HealthDef);
	RelevantAttributesToCapture.Add(Damage().PhysicalDamageReductionDef);
	RelevantAttributesToCapture.Add(Damage().EnvironmentalDamageReductionDef);
	RelevantAttributesToCapture.Add(Damage().FlatDamageReductionDef);
	RelevantAttributesToCapture.Add(Damage().AttackPowerDef);
	RelevantAttributesToCapture.Add(Damage().SpellPowerDef);
	RelevantAttributesToCapture.Add(Damage().CriticalChanceDef);
	RelevantAttributesToCapture.Add(Damage().CriticalDamageDef);
}

void UCalculateDamage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	UAbilitySystemComponent* TargetAbilitySystemComponent = ExecutionParams.GetTargetAbilitySystemComponent();
	UAbilitySystemComponent* SourceAbilitySystemComponent = ExecutionParams.GetSourceAbilitySystemComponent();
	AActor* SourceActor = SourceAbilitySystemComponent ? SourceAbilitySystemComponent->AvatarActor : nullptr;
	AActor* TargetActor = TargetAbilitySystemComponent ? TargetAbilitySystemComponent->AvatarActor : nullptr;

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	// --------------------------------------------------------------------------------------------
	// Target relevant parameters
	float Health = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Damage().HealthDef, EvaluationParameters, Health);
	float PhysicalDamageReduction = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Damage().PhysicalDamageReductionDef, EvaluationParameters, PhysicalDamageReduction);
	float EnvironmentalDamageReduction = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Damage().EnvironmentalDamageReductionDef, EvaluationParameters, EnvironmentalDamageReduction);
	float FlatDamageReduction = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Damage().FlatDamageReductionDef, EvaluationParameters, FlatDamageReduction);

	// Source relevant parameters
	float AttackPower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Damage().AttackPowerDef, EvaluationParameters, AttackPower);
	float SpellPower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Damage().SpellPowerDef, EvaluationParameters, SpellPower);
	float CriticalChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Damage().CriticalChanceDef, EvaluationParameters, CriticalChance);
	float CriticalDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Damage().CriticalDamageDef, EvaluationParameters, CriticalDamage);

	// --------------------------------------
	//	Damage Done
	//	
	// --------------------------------------

	float DamageDone = AttackPower  * (1 - PhysicalDamageReduction) * (1 - FlatDamageReduction);

	if (DamageDone > 0.f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(Damage().HealthProperty, EGameplayModOp::Additive, -DamageDone));
	}
}