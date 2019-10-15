// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MOBAAttributeSet.h"
#include "BasicAttackCooldownOffhand.generated.h"

/**
 * 
 */
UCLASS()
class MOBA_API UBasicAttackCooldownOffhand : public UGameplayModMagnitudeCalculation
{
	GENERATED_UCLASS_BODY()
public:
	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;
};
