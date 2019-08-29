// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MOBAAttributeSet.h"
#include "BasicAttackCooldown.generated.h"

/**
 * 
 */
UCLASS(BlueprintType, Blueprintable, Abstract)
class MOBA_API UBasicAttackCooldown : public UGameplayModMagnitudeCalculation
{
	GENERATED_UCLASS_BODY()

public:
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
