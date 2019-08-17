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
#include "ConstructorHelpers.h"
#include "ExperiencePerLevel.h"
#include "MOBAAttributeSet.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHealthChange, FGameplayAttributeData, Health, FGameplayAttributeData, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FManaChange, FGameplayAttributeData, Mana, FGameplayAttributeData, MaxMana);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLevelChange, FGameplayAttributeData, Level, FGameplayAttributeData, MaxLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FExperienceChange, FGameplayAttributeData, Experience, FGameplayAttributeData, MaxExperience);

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

	UPROPERTY(Category = "Attributes | Mana", EditAnywhere, BlueprintReadWrite)
		FGameplayAttributeData Mana;

	UPROPERTY(Category = "Attributes | Mana", EditAnywhere, BlueprintReadWrite)
		FGameplayAttributeData MaxMana;

	UPROPERTY(Category = "Attributes | Experience", EditAnywhere, BlueprintReadWrite)
		FGameplayAttributeData Level;

	UPROPERTY(Category = "Attributes | Experience", EditAnywhere, BlueprintReadWrite)
		FGameplayAttributeData MaxLevel;
	
	UPROPERTY(Category = "Attributes | Experience", EditAnywhere, BlueprintReadWrite)
		FGameplayAttributeData Experience;

	UPROPERTY(Category = "Attributes | Experience", EditAnywhere, BlueprintReadWrite)
		FGameplayAttributeData MaxExperience;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes | Experience", meta = (AllowPrivateAccess = "true"))
		class UDataTable* ExperiencePerLevelData;

	UFUNCTION(BlueprintImplementableEvent)
		void BP_HealthChange();

	UFUNCTION(BlueprintImplementableEvent)
		void BP_ManaChange();

	UFUNCTION(BlueprintImplementableEvent)
		void BP_LevelChange();

	UFUNCTION(BlueprintImplementableEvent)
		void BP_ExperienceChange();

	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data);
	FGameplayAttribute HealthAttribute();
	FGameplayAttribute ManaAttribute();
	FGameplayAttribute LevelAttribute();
	FGameplayAttribute ExperienceAttribute();

	FHealthChange HealthChange;
	FManaChange ManaChange;
	FLevelChange LevelChange;
	FExperienceChange ExperienceChange;
	
};

