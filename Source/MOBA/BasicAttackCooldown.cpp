// Fill out your copyright notice in the Description page of Project Settings.


#include "BasicAttackCooldown.h"
#include "MOBAAttributeSet.h"
#include "MOBACharacter.h"
#include "GameplayEffect.h"
#include "GameplayEffectExecutionCalculation.h"

UBasicAttackCooldown::UBasicAttackCooldown(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{}

float UBasicAttackCooldown::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const {
	FGameplayEffectContextHandle SpecHandle = Spec.GetContext();
	UAbilitySystemComponent* AbilitySystem= SpecHandle.GetInstigatorAbilitySystemComponent();
	if (AbilitySystem)
	{
		AMOBACharacter* MyCharacter = Cast<AMOBACharacter>(AbilitySystem->GetOwner());
		if (MyCharacter) return (1 / (MyCharacter->AttributeSet->AttackSpeed.GetCurrentValue()));
		else return 0.0f;
	}
	else return 0.0f;

}