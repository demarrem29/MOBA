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
#include "ObjectMacros.h"
#include "MOBAAttributeSet.generated.h"

/**
 * 
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FHealthChange, FGameplayAttributeData, Health, FGameplayAttributeData, MaxHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHealthRegenChange, FGameplayAttributeData, HealthRegen);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHealingModifierChange, FGameplayAttributeData, HealingModifier);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FManaChange, FGameplayAttributeData, Mana, FGameplayAttributeData, MaxMana);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FManaRegenChange, FGameplayAttributeData, ManaRegen);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FLevelChange, FGameplayAttributeData, Level, FGameplayAttributeData, MaxLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FExperienceChange, FGameplayAttributeData, Experience, FGameplayAttributeData, MaxExperience);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAttackPowerChange, FGameplayAttributeData, AttackPower);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpellPowerChange, FGameplayAttributeData, SpellPower);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FMainHandChange, FGameplayAttributeData, MainHandAttackSpeed, FGameplayAttributeData, MainHandMinDamage, FGameplayAttributeData, MainHandMaxDamage, FGameplayAttributeData, MainHandAttackRange);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOffHandChange, FGameplayAttributeData, OffHandAttackSpeed, FGameplayAttributeData, OffHandMinDamage, FGameplayAttributeData, OffHandMaxDamage, FGameplayAttributeData, OffHandAttackRange);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBonusAttackSpeedChange, FGameplayAttributeData, BonusAttackSpeed);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCriticalChanceChange, FGameplayAttributeData, CriticalChance);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FCriticalDamageChange, FGameplayAttributeData, CriticalDamage);
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

	UPROPERTY(Category = "Attributes | Health", EditAnywhere, BlueprintReadWrite) // Increased/Decreased Healing Ratio
		FGameplayAttributeData HealingModifier;

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

	UPROPERTY(Category = "Attributes | Offense", EditAnywhere, BlueprintReadWrite)
		FGameplayAttributeData AttackPower;

	UPROPERTY(Category = "Attributes | Offense", EditAnywhere, BlueprintReadWrite)
		FGameplayAttributeData SpellPower;
	
	UPROPERTY(Category = "Attributes | Offense", EditAnywhere, BlueprintReadWrite) // Attack speed from items or abilities
		FGameplayAttributeData MainHandMinDamage;

	UPROPERTY(Category = "Attributes | Offense", EditAnywhere, BlueprintReadWrite) // Attack speed from items or abilities
		FGameplayAttributeData MainHandMaxDamage;

	UPROPERTY(Category = "Attributes | Offense", EditAnywhere, BlueprintReadWrite) // Attack speed from items or abilities
		FGameplayAttributeData MainHandAttackSpeed;

	UPROPERTY(Category = "Attributes | Offense", EditAnywhere, BlueprintReadWrite) // Attack range from items or abilities
		FGameplayAttributeData MainHandAttackRange;

	UPROPERTY(Category = "Attributes | Offense", EditAnywhere, BlueprintReadWrite) // Attack speed from items or abilities
		FGameplayAttributeData OffHandMinDamage;

	UPROPERTY(Category = "Attributes | Offense", EditAnywhere, BlueprintReadWrite) // Attack speed from items or abilities
		FGameplayAttributeData OffHandMaxDamage;

	UPROPERTY(Category = "Attributes | Offense", EditAnywhere, BlueprintReadWrite) // Extra attack speed from items or abilities
		FGameplayAttributeData OffHandAttackSpeed;
	
	UPROPERTY(Category = "Attributes | Offense", EditAnywhere, BlueprintReadWrite) // Attack range from items or abilities
		FGameplayAttributeData OffHandAttackRange;

	UPROPERTY(Category = "Attributes | Offense", EditAnywhere, BlueprintReadWrite) // Extra attack speed from items or abilities
		FGameplayAttributeData BonusAttackSpeed;

	UPROPERTY(Category = "Attributes | Offense", EditAnywhere, BlueprintReadWrite)
		FGameplayAttributeData CriticalChance;

	UPROPERTY(Category = "Attributes | Offense", EditAnywhere, BlueprintReadWrite)
		FGameplayAttributeData CriticalDamage;
	
	UPROPERTY(Category = "Attributes | Defense", EditAnywhere, BlueprintReadWrite) // Used to calculate PhysicalDamageReduction
		FGameplayAttributeData Armor;

	UPROPERTY(Category = "Attributes | Defense", EditAnywhere, BlueprintReadWrite)
		FGameplayAttributeData PhysicalDamageReduction;

	UPROPERTY(Category = "Attributes | Defense", EditAnywhere, BlueprintReadWrite) // Used to calculate EnvironmentalDamageReduction
		FGameplayAttributeData EnvironmentalResistance;

	UPROPERTY(Category = "Attributes | Defense", EditAnywhere, BlueprintReadWrite)
		FGameplayAttributeData EnvironmentalDamageReduction;

	UPROPERTY(Category = "Attributes | Defense", EditAnywhere, BlueprintReadWrite) // Provided by gameplay effects
		FGameplayAttributeData FlatDamageReduction;

	UPROPERTY(Category = "Attributes | Movement", EditAnywhere, BlueprintReadWrite) // Units traveled per second
		FGameplayAttributeData MovementSpeed;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attributes | Experience", meta = (AllowPrivateAccess = "true"))
		class UDataTable* ExperiencePerLevelData;

	float CalculateDamageReduction(float ResistanceStat);
	
	// Event handlers for when attributes change
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data);
	
	// Attributes
	FGameplayAttribute HealthAttribute();
	FGameplayAttribute MaxHealthAttribute();
	FGameplayAttribute HealthRegenAttribute();
	FGameplayAttribute HealingModifierAttribute();
	FGameplayAttribute ManaAttribute();
	FGameplayAttribute ManaRegenAttribute();
	FGameplayAttribute LevelAttribute();
	FGameplayAttribute ExperienceAttribute();
	FGameplayAttribute AttackPowerAttribute();
	FGameplayAttribute SpellPowerAttribute();
	FGameplayAttribute MainHandAttackSpeedAttribute();
	FGameplayAttribute OffHandAttackSpeedAttribute();
	FGameplayAttribute BonusAttackSpeedAttribute();
	FGameplayAttribute MainHandMinDamageAttribute();
	FGameplayAttribute MainHandMaxDamageAttribute();
	FGameplayAttribute OffHandMinDamageAttribute();
	FGameplayAttribute OffHandMaxDamageAttribute();
	FGameplayAttribute CriticalChanceAttribute();
	FGameplayAttribute CriticalDamageAttribute();
	FGameplayAttribute MainHandAttackRangeAttribute();
	FGameplayAttribute OffHandAttackRangeAttribute();
	FGameplayAttribute ArmorAttribute();
	FGameplayAttribute PhysicalDamageReductionAttribute();
	FGameplayAttribute EnvironmentalResistanceAttribute();
	FGameplayAttribute EnvironmentalDamageReductionAttribute();
	FGameplayAttribute FlatDamageReductionAttribute();
	FGameplayAttribute MovementSpeedAttribute();

	// Attribute Delegates
	FHealthChange HealthChange;
	FHealthRegenChange HealthRegenChange;
	FHealingModifierChange HealingModifierChange;
	FManaChange ManaChange;
	FManaRegenChange ManaRegenChange;
	FLevelChange LevelChange;
	FExperienceChange ExperienceChange;
	FAttackPowerChange AttackPowerChange;
	FSpellPowerChange SpellPowerChange;
	FMainHandChange MainHandChange;
	FOffHandChange OffHandChange;
	FBonusAttackSpeedChange BonusAttackSpeedChange;
	FCriticalChanceChange CriticalChanceChange;
	FCriticalDamageChange CriticalDamageChange;
	FArmorChange ArmorChange;
	FPhysicalDamageReductionChange PhysicalDamageReductionChange;
	FEnvironmentalResistanceChange EnvironmentalResistanceChange;
	FEnvironmentalDamageReductionChange EnvironmentalDamageReductionChange;
	FFlatDamageReductionChange FlatDamageReductionChange;
	FMovementSpeedChange MovementSpeedChange;
};