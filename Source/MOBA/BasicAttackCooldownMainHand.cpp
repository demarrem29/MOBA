// Fill out your copyright notice in the Description page of Project Settings.


#include "BasicAttackCooldownMainHand.h"
#include "MOBAAttributeSet.h"
#include "MOBACharacter.h"
#include "GameplayEffect.h"
#include "GameplayEffectExecutionCalculation.h"

UBasicAttackCooldownMainHand::UBasicAttackCooldownMainHand(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

float UBasicAttackCooldownMainHand::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const {
	FGameplayEffectContextHandle SpecHandle = Spec.GetContext();
	UAbilitySystemComponent* AbilitySystem = SpecHandle.GetInstigatorAbilitySystemComponent();
	if (AbilitySystem)
	{
		AMOBACharacter* MyCharacter = Cast<AMOBACharacter>(AbilitySystem->GetOwner());
		if (MyCharacter) // Main hand attack speed * bonus attack speed multiplier, converted to time
		{
			// Check if the character is dual-wielding, add 15% attack speed bonus if so.			
			return (1 / (MyCharacter->AttributeSet->MainHandAttackSpeed.GetCurrentValue() * (1 + MyCharacter->AttributeSet->BonusAttackSpeed.GetCurrentValue())));
		}
		else return 0.0f;
	}
	else return 0.0f;
}