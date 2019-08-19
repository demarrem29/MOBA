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
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHealthRegenChange, FGameplayAttributeData, HealthRegen);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FManaChange, FGameplayAttributeData, Mana, FGameplayAttributeData, MaxMana);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FManaRegenChange, FGameplayAttributeData, ManaRegen);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLevelChange, FGameplayAttributeData, Level, FGameplayAttributeData, MaxLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FExperienceChange, FGameplayAttributeData, Experience, FGameplayAttributeData, MaxExperience);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAttackPowerChange, FGameplayAttributeData, AttackPower);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpellPowerChange, FGameplayAttributeData, SpellPower);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAttackSpeedChange, FGameplayAttributeData, AttackSpeed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCriticalChanceChange, FGameplayAttributeData, CriticalChance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCriticalDamageChange, FGameplayAttributeData, CriticalDamage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAttackRangeChange, FGameplayAttributeData, AttackRange);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FArmorChange, FGameplayAttributeData, Armor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPhysicalDamageReductionChange, FGameplayAttributeData, PhysicalDamageReduction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnvironmentalResistanceChange, FGameplayAttributeData, EnvironmentalResistance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEnvironmentalDamageReductionChange, FGameplayAttributeData, EnvironmentalDamageReduction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFlatDamageReductionChange, FGameplayAttributeData, FlatDamageReduction);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMovementSpeedChange, FGameplayAttributeData, MovementSpeed);

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

	UPROPERTY(Category = "Attributes | Health", EditAnywhere, BlueprintReadWrite) // Health per 5 Seconds
		FGameplayAttributeData HealthRegen;

	UPROPERTY(Category = "Attributes | Mana", EditAnywhere, BlueprintReadWrite)
		FGameplayAttributeData Mana;

	UPROPERTY(Category = "Attributes | Mana", EditAnywhere, BlueprintReadWrite)
		FGameplayAttributeData MaxMana;

	UPROPERTY(Category = "Attributes | Mana", EditAnywhere, BlueprintReadWrite)
		FGameplayAttributeData ManaRegen;

	UPROPERTY(Category = "Attributes | Experience", EditAnywhere, BlueprintReadWrite)
		FGameplayAttributeData Level;

	UPROPERTY(Category = "Attributes | Experience", EditAnywhere, BlueprintReadWrite)
		FGameplayAttributeData MaxLevel;
	
	UPROPERTY(Category = "Attributes | Experience", EditAnywhere, BlueprintReadWrite)
		FGameplayAttributeData Experience;

	UPROPERTY(Category = "Attributes | Experience", EditAnywhere, BlueprintReadWrite)
		FGameplayAttributeData MaxExperience;

	UPROPERTY(Category = "Attributes | Combat", EditAnywhere, BlueprintReadWrite)
		FGameplayAttributeData AttackPower;

	UPROPERTY(Category = "Attributes | Combat", EditAnywhere, BlueprintReadWrite)
		FGameplayAttributeData SpellPower;

	UPROPERTY(Category = "Attributes | Combat", EditAnywhere, BlueprintReadWrite) // Attack frequency. For example, default is 0.7 attacks per second
		FGameplayAttributeData AttackSpeed;

	UPROPERTY(Category = "Attributes | Combat", EditAnywhere, BlueprintReadWrite)
		FGameplayAttributeData CriticalChance;

	UPROPERTY(Category = "Attributes | Combat", EditAnywhere, BlueprintReadWrite)
		FGameplayAttributeData CriticalDamage;

	UPROPERTY(Category = "Attributes | Combat", EditAnywhere, BlueprintReadWrite)
		FGameplayAttributeData AttackRange;
	
	UPROPERTY(Category = "Attributes | Combat", EditAnywhere, BlueprintReadWrite) // Used to calculate PhysicalDamageReduction
		FGameplayAttributeData Armor;

	UPROPERTY(Category = "Attributes | Combat", EditAnywhere, BlueprintReadWrite)
		FGameplayAttributeData PhysicalDamageReduction;

	UPROPERTY(Category = "Attributes | Combat", EditAnywhere, BlueprintReadWrite) // Used to calculate EnvironmentalDamageReduction
		FGameplayAttributeData EnvironmentalResistance;

	UPROPERTY(Category = "Attributes | Combat", EditAnywhere, BlueprintReadWrite)
		FGameplayAttributeData EnvironmentalDamageReduction;

	UPROPERTY(Category = "Attributes | Combat", EditAnywhere, BlueprintReadWrite) // Provided by gameplay effects
		FGameplayAttributeData FlatDamageReduction;

	UPROPERTY(Category = "Attributes | Movement", EditAnywhere, BlueprintReadWrite) // Units traveled per second
		FGameplayAttributeData MovementSpeed;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes | Experience", meta = (AllowPrivateAccess = "true"))
		class UDataTable* ExperiencePerLevelData;

	float CalculateDamageReduction(float ResistanceStat);
	
	// Event handlers for when attributes change
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data);
	
	// Attributes
	FGameplayAttribute HealthAttribute();
	FGameplayAttribute HealthRegenAttribute();
	FGameplayAttribute ManaAttribute();
	FGameplayAttribute ManaRegenAttribute();
	FGameplayAttribute LevelAttribute();
	FGameplayAttribute ExperienceAttribute();
	FGameplayAttribute AttackPowerAttribute();
	FGameplayAttribute SpellPowerAttribute();
	FGameplayAttribute AttackSpeedAttribute();
	FGameplayAttribute CriticalChanceAttribute();
	FGameplayAttribute CriticalDamageAttribute();
	FGameplayAttribute AttackRangeAttribute();
	FGameplayAttribute ArmorAttribute();
	FGameplayAttribute PhysicalDamageReductionAttribute();
	FGameplayAttribute EnvironmentalResistanceAttribute();
	FGameplayAttribute EnvironmentalDamageReductionAttribute();
	FGameplayAttribute FlatDamageReductionAttribute();
	FGameplayAttribute MovementSpeedAttribute();

	// Attribute Delegates
	FHealthChange HealthChange;
	FHealthRegenChange HealthRegenChange;
	FManaChange ManaChange;
	FManaRegenChange ManaRegenChange;
	FLevelChange LevelChange;
	FExperienceChange ExperienceChange;
	FAttackPowerChange AttackPowerChange;
	FSpellPowerChange SpellPowerChange;
	FAttackSpeedChange AttackSpeedChange;
	FCriticalChanceChange CriticalChanceChange;
	FCriticalDamageChange CriticalDamageChange;
	FAttackRangeChange AttackRangeChange;
	FArmorChange ArmorChange;
	FPhysicalDamageReductionChange PhysicalDamageReductionChange;
	FEnvironmentalResistanceChange EnvironmentalResistanceChange;
	FEnvironmentalDamageReductionChange EnvironmentalDamageReductionChange;
	FFlatDamageReductionChange FlatDamageReductionChange;
	FMovementSpeedChange MovementSpeedChange;
};