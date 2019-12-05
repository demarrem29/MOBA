// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "CalculateHealing.generated.h"

/**
 * 
 */
UCLASS()
class MOBA_API UCalculateHealing : public UGameplayEffectExecutionCalculation
{
	GENERATED_UCLASS_BODY()

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
