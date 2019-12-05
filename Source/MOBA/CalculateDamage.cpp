// Fill out your copyright notice in the Description page of Project Settings.


#include "CalculateDamage.h"
#include "MOBAAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "MOBAGameplayAbility.h"
#include "Engine/BlueprintGeneratedClass.h"

struct DamageStatics
{
	// Target's Relevant Stats
	DECLARE_ATTRIBUTE_CAPTUREDEF(Health);
	DECLARE_ATTRIBUTE_CAPTUREDEF(MaxHealth); // Source and target
	DECLARE_ATTRIBUTE_CAPTUREDEF(HealthRegen);
	DECLARE_ATTRIBUTE_CAPTUREDEF(HealingModifier); // Source and target
	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalDamageReduction);
	DECLARE_ATTRIBUTE_CAPTUREDEF(EnvironmentalDamageReduction);
	DECLARE_ATTRIBUTE_CAPTUREDEF(FlatDamageReduction);
	// Attacker's Relevant Stats
	DECLARE_ATTRIBUTE_CAPTUREDEF(AttackPower);
	DECLARE_ATTRIBUTE_CAPTUREDEF(SpellPower);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalDamage);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Level);
	DECLARE_ATTRIBUTE_CAPTUREDEF(MainHandMinDamage);
	DECLARE_ATTRIBUTE_CAPTUREDEF(MainHandMaxDamage);
	DECLARE_ATTRIBUTE_CAPTUREDEF(OffHandMinDamage);
	DECLARE_ATTRIBUTE_CAPTUREDEF(OffHandMaxDamage);

	

	DamageStatics()
	{
		// Capture the Target's Health attribute. Do not snapshot it, because we want to use the health value at the moment we apply the execution.
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, Health, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, MaxHealth, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, HealthRegen, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, HealingModifier, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, PhysicalDamageReduction, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, EnvironmentalDamageReduction, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, FlatDamageReduction, Target, false);

		// Capture the Source's Damage Stats. We do want to snapshot this at the moment we create the GameplayEffectSpec that will execute the damage.
		// (imagine we fire a projectile: we create the GE Spec when the projectile is fired. When it hits the target, we want to use the AttackPower at the moment
		// the projectile was launched, not when it hits).
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, MaxHealth, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, Level, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, HealingModifier, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, AttackPower, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, SpellPower, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, CriticalChance, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, CriticalDamage, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, MainHandMinDamage, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, MainHandMaxDamage, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, OffHandMinDamage, Source, true);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, OffHandMaxDamage, Source, true);
	}
};

DamageStatics& Source()
{
	static DamageStatics It;
	return It;
}

DamageStatics& Target()
{
	static DamageStatics It;
	return It;
}

UCalculateDamage::UCalculateDamage(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Target
	RelevantAttributesToCapture.Add(Target().HealthDef);
	RelevantAttributesToCapture.Add(Target().MaxHealthDef);
	RelevantAttributesToCapture.Add(Target().HealthRegenDef);
	RelevantAttributesToCapture.Add(Target().HealingModifierDef);
	RelevantAttributesToCapture.Add(Target().PhysicalDamageReductionDef);
	RelevantAttributesToCapture.Add(Target().EnvironmentalDamageReductionDef);
	RelevantAttributesToCapture.Add(Target().FlatDamageReductionDef);
	// Source
	RelevantAttributesToCapture.Add(Source().MaxHealthDef);
	RelevantAttributesToCapture.Add(Source().LevelDef);
	RelevantAttributesToCapture.Add(Source().HealingModifierDef);
	RelevantAttributesToCapture.Add(Source().AttackPowerDef);
	RelevantAttributesToCapture.Add(Source().SpellPowerDef);
	RelevantAttributesToCapture.Add(Source().CriticalChanceDef);
	RelevantAttributesToCapture.Add(Source().CriticalDamageDef);
	RelevantAttributesToCapture.Add(Source().MainHandMinDamageDef);
	RelevantAttributesToCapture.Add(Source().MainHandMaxDamageDef);
	RelevantAttributesToCapture.Add(Source().OffHandMinDamageDef);
	RelevantAttributesToCapture.Add(Source().OffHandMaxDamageDef);
}

void UCalculateDamage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{	
	// Get Gameplay Effect Spec and Source Ability Data
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FGameplayEffectContextHandle Handle = Spec.GetContext();
	const UGameplayAbility* SourceAbility = Handle.GetAbility();
	const UMOBAGameplayAbility* MOBASourceAbility = Cast<UMOBAGameplayAbility>(SourceAbility);
	UClass* AbilityData;
	UMOBAAbilityData* MOBAAbilityData;
	bool bIsHealthRegen = false;

	if (MOBASourceAbility) 
	{
		AbilityData = MOBASourceAbility->AbilityData;
		UObject* AbilityDataObj = AbilityData->ClassDefaultObject;
		MOBAAbilityData = Cast<UMOBAAbilityData>(AbilityDataObj);
		if (!MOBAAbilityData) return;
		if (MOBAAbilityData->GetFName() == FName("Default__BP_AD_HealthRegen_C")) 
		{
			bIsHealthRegen = true;
		}
	}
	else return;

	// Get Source and target data
	UAbilitySystemComponent* TargetAbilitySystemComponent = ExecutionParams.GetTargetAbilitySystemComponent();
	UAbilitySystemComponent* SourceAbilitySystemComponent = ExecutionParams.GetSourceAbilitySystemComponent();
	AActor* SourceActor = SourceAbilitySystemComponent ? SourceAbilitySystemComponent->AvatarActor : nullptr;
	AActor* TargetActor = TargetAbilitySystemComponent ? TargetAbilitySystemComponent->AvatarActor : nullptr;

	
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	// --------------------------------------------------------------------------------------------
	// Target relevant parameters
	float Health = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Target().HealthDef, EvaluationParameters, Health);
	float MaxHealth = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Target().MaxHealthDef, EvaluationParameters, MaxHealth);
	float HealthRegen = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Target().HealthRegenDef, EvaluationParameters, HealthRegen);
	float TargetHealingModifier = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Target().HealingModifierDef, EvaluationParameters, TargetHealingModifier);
	float PhysicalDamageReduction = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Target().PhysicalDamageReductionDef, EvaluationParameters, PhysicalDamageReduction);
	float EnvironmentalDamageReduction = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Target().EnvironmentalDamageReductionDef, EvaluationParameters, EnvironmentalDamageReduction);
	float FlatDamageReduction = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Target().FlatDamageReductionDef, EvaluationParameters, FlatDamageReduction);

	// Source relevant parameters
	float SourceHealingModifier = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Source().HealingModifierDef, EvaluationParameters, SourceHealingModifier);
	float SourceMaxHealth = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Source().MaxHealthDef, EvaluationParameters, SourceMaxHealth);
	float Level = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Source().LevelDef, EvaluationParameters, Level);
	float AttackPower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Source().AttackPowerDef, EvaluationParameters, AttackPower);
	float SpellPower = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Source().SpellPowerDef, EvaluationParameters, SpellPower);
	float CriticalChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Source().CriticalChanceDef, EvaluationParameters, CriticalChance);
	float CriticalDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Source().CriticalDamageDef, EvaluationParameters, CriticalDamage);
	float MainHandMinDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Source().MainHandMinDamageDef, EvaluationParameters, MainHandMinDamage);
	float MainHandMaxDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Source().MainHandMaxDamageDef, EvaluationParameters, MainHandMaxDamage);
	float OffHandMinDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Source().MainHandMinDamageDef, EvaluationParameters, OffHandMinDamage);
	float OffHandMaxDamage = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Source().MainHandMaxDamageDef, EvaluationParameters, OffHandMaxDamage);

	// --------------------------------------
	//	Damage Done
	//	
	// --------------------------------------

	float DamageDone = 0.0f;
	float HealingDone = 0.0f;
	switch (MOBAAbilityData->DamageType) 
	{
	case EMOBADamageType::Physical: DamageDone = (MOBAAbilityData->BaseValue + MOBAAbilityData->AttackPowerRatio * AttackPower + MOBAAbilityData->SpellPowerRatio * SpellPower) * (1 - PhysicalDamageReduction) * (1 - FlatDamageReduction);
		break;
	case EMOBADamageType::Environmental: DamageDone = (MOBAAbilityData->BaseValue + MOBAAbilityData->AttackPowerRatio * AttackPower + MOBAAbilityData->SpellPowerRatio * SpellPower) * (1 - EnvironmentalDamageReduction) * (1 - FlatDamageReduction);
		break;
	case EMOBADamageType::TrueDamage: DamageDone = (MOBAAbilityData->BaseValue + MOBAAbilityData->AttackPowerRatio * AttackPower + MOBAAbilityData->SpellPowerRatio * SpellPower) * (1 - FlatDamageReduction);
		break;
	case EMOBADamageType::Heal: if (bIsHealthRegen) 
	{
		HealingDone = (HealthRegen / 5) * TargetHealingModifier;
	}
								else 
	{
		HealingDone = (MOBAAbilityData->BaseValue + MOBAAbilityData->AttackPowerRatio * AttackPower + MOBAAbilityData->SpellPowerRatio * SpellPower) * (TargetHealingModifier);
	}
		break;
	default: break;
	} 

	if (DamageDone > 0.f)
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(Target().HealthProperty, EGameplayModOp::Additive, -DamageDone));
	}
	if (HealingDone > 0.f) 
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(Target().HealthProperty, EGameplayModOp::Additive, HealingDone));
	}
}