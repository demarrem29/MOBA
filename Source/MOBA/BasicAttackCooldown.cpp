// Fill out your copyright notice in the Description page of Project Settings.


#include "BasicAttackCooldown.h"
#include "MOBAAttributeSet.h"
#include "MOBACharacter.h"
#include "GameplayEffect.h"
#include "GameplayEffectExecutionCalculation.h"

UBasicAttackCooldown::UBasicAttackCooldown(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) 
{
	RelevantAttributesToCapture.Add(AttackSpeedCaptureDefinition);
}

float UBasicAttackCooldown::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	FGameplayEffectContextHandle SpecHandle = Spec.GetContext();
	AMOBACharacter* MyCharacter = Cast<AMOBACharacter>(SpecHandle.GetInstigator());
	if (!MyCharacter) 
	{
		UE_LOG(LogTemp, Error, TEXT("Error, could not get a valid pointer to a character"));
		return 0.0f;
	}
	else 
	{
		return (1 / (MyCharacter->AttributeSet->AttackSpeed.GetCurrentValue()));
	}

}