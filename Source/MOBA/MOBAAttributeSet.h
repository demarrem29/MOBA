// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "GameFramework/Character.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "GameplayEffect.h"
#include "GameplayTagsModule.h"
#include "GameplayEffectExtension.h"
#include "MOBAAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class MOBA_API UMOBAAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UMOBAAttributeSet();
	UPROPERTY(Category = "Attributes | Health", EditAnywhere, BlueprintReadWrite)
	FGameplayAttributeData Health;

	UPROPERTY(Category = "Attributes | Health", EditAnywhere, BlueprintReadWrite)
	FGameplayAttributeData MaxHealth;

	UPROPERTY(Category = "Attributes | Health", EditAnywhere, BlueprintReadWrite)
		FGameplayAttributeData Mana;

	UPROPERTY(Category = "Attributes | Health", EditAnywhere, BlueprintReadWrite)
		FGameplayAttributeData MaxMana;

	UPROPERTY(Category = "Attributes | Health", EditAnywhere, BlueprintReadWrite)
	FGameplayAttributeData Level;

	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data);
	FGameplayAttribute HealthAttribute();
	FGameplayAttribute ManaAttribute();
	FGameplayAttribute LevelAttribute();
};
