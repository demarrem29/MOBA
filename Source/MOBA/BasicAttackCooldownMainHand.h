// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MOBAAttributeSet.h"
#include "BasicAttackCooldownMainHand.generated.h"

/**
 * 
 */
UCLASS()
class MOBA_API UBasicAttackCooldownMainHand : public UGameplayModMagnitudeCalculation
{
	GENERATED_UCLASS_BODY()

public:
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
