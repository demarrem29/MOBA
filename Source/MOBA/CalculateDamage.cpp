// Fill out your copyright notice in the Description page of Project Settings.


#include "CalculateDamage.h"
#include "MOBAAttributeSet.h"
#include "AbilitySystemComponent.h"

struct DamageStatics
{
	// Target's Defense Stats
	DECLARE_ATTRIBUTE_CAPTUREDEF(CurrentHealth);
	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalDamageReduction);
	DECLARE_ATTRIBUTE_CAPTUREDEF(EnvironmentalDamageReduction);
	DECLARE_ATTRIBUTE_CAPTUREDEF(FlatDamageReduction);
	// Attacker's Offense Stats
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalDamage);
	

	DamageStatics()
	{
		// Capture the Target's Health attribute. Do not snapshot it, because we want to use the health value at the moment we apply the execution.
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, CurrentHealth, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, PhysicalDamageReduction, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, EnvironmentalDamageReduction, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, FlatDamageReduction, Target, false);

		// Capture the Source's Damage Stats. We do want to snapshot this at the moment we create the GameplayEffectSpec that will execute the damage.
		// (imagine we fire a projectile: we create the GE Spec when the projectile is fired. When it hits the target, we want to use the AttackPower at the moment
		// the projectile was launched, not when it hits).
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, CriticalChance, Source, True);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, CriticalDamage, Source, True); 
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
	RelevantAttributesToCapture.Add(Damage().CurrentHealthDef);
	RelevantAttributesToCapture.Add(Damage().PhysicalDamageReductionDef);
	RelevantAttributesToCapture.Add(Damage().EnvironmentalDamageReductionDef);
	RelevantAttributesToCapture.Add(Damage().FlatDamageReductionDef);
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
	float CurrentHealth = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Damage().CurrentHealthDef, EvaluationParameters, CurrentHealth);
	float PhysicalDamageReduction = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Damage().PhysicalDamageReductionDef, EvaluationParameters, PhysicalDamageReduction);

	// Source relevant parameters
	float DamageMin = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Damage().DamageMinDef, EvaluationParameters, DamageMin);
	float DamageMax = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Damage().DamageMaxDef, EvaluationParameters, DamageMax);

	// --------------------------------------
	//	Damage Done
	//	
	// --------------------------------------

	float DamageDone = DamageMin + (rand() % int(DamageMax)) * (1 - PhysicalResistance);

	if (DamageDone > 0.f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(Damage().CurrentHealthProperty, EGameplayModOp::Additive, -DamageDone));
	}
}