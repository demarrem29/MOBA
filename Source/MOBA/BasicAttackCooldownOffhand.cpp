// Fill out your copyright notice in the Description page of Project Settings.


#include "BasicAttackCooldownOffhand.h"
#include "MOBAAttributeSet.h"
#include "MOBACharacter.h"
#include "GameplayEffect.h"
#include "GameplayEffectExecutionCalculation.h"

UBasicAttackCooldownOffhand::UBasicAttackCooldownOffhand(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

float UBasicAttackCooldownOffhand::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const {
	FGameplayEffectContextHandle SpecHandle = Spec.GetContext();
	UAbilitySystemComponent* AbilitySystem = SpecHandle.GetInstigatorAbilitySystemComponent();
	if (AbilitySystem)
	{
		AMOBACharacter* MyCharacter = Cast<AMOBACharacter>(AbilitySystem->GetOwner());
		if (MyCharacter) // Main hand attack speed * bonus attack speed multiplier, converted to time
		{
			return (1 / (MyCharacter->AttributeSet->OffHandAttackSpeed.GetCurrentValue() * (1 + MyCharacter->AttributeSet->BonusAttackSpeed.GetCurrentValue())*1.15f));
		}
		else return 0.0f;
	}
	else return 0.0f;
}