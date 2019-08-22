// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "CalculateDamage.generated.h"

/**
 * 
 */
UCLASS()
class MOBA_API UCalculateDamage : public UGameplayEffectExecutionCalculation
{
	GENERATED_UCLASS_BODY()
	
	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, OUT FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;

};
