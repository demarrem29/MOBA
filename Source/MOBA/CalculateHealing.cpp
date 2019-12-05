// Fill out your copyright notice in the Description page of Project Settings.


#include "CalculateHealing.h"
#include "MOBAGameplayAbility.h"
#include "MOBAAttributeSet.h"

struct HealingStatics 
{
	DECLARE_ATTRIBUTE_CAPTUREDEF(HealingModifier);
	DECLARE_ATTRIBUTE_CAPTUREDEF(HealthRegen);

	HealingStatics() 
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, HealingModifier, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(UMOBAAttributeSet, HealthRegen, Target, false);
	}
};

HealingStatics& Source()
{
	static HealingStatics It;
	return It;
}

HealingStatics& Target()
{
	static HealingStatics It;
	return It;
}

UCalculateHealing::UCalculateHealing(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Target
	RelevantAttributesToCapture.Add(Target().HealingModifierDef);
	RelevantAttributesToCapture.Add(Target().HealthRegenDef);
}

void UCalculateHealing::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const 
{
	// Get Gameplay Effect Spec and Source Ability Data
	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FGameplayEffectContextHandle Handle = Spec.GetContext();
	const UGameplayAbility* SourceAbility = Handle.GetAbility();
	const UMOBAGameplayAbility* MOBASourceAbility = Cast<UMOBAGameplayAbility>(SourceAbility);
	UClass* AbilityData;
	UMOBAAbilityData* MOBAAbilityData;
	if (MOBASourceAbility)
	{
		AbilityData = MOBASourceAbility->AbilityData;
		UObject* AbilityDataObj = AbilityData->ClassDefaultObject;
		MOBAAbilityData = Cast<UMOBAAbilityData>(AbilityDataObj);
		if (!MOBAAbilityData) return;
	}
	else return;
}