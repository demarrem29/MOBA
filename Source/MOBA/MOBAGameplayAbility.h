// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "MOBACharacter.h"
#include "AIController.h"
#include "MOBAGameplayAbility.generated.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EMOBADamageType : uint8
{

	None UMETA(DisplayName = "No Damage"),
	Physical UMETA(DisplayName = "Physical Damage"),
	Environmental UMETA(DisplayName = "Environmental Damage"),
	TrueDamage UMETA(DisplayName = "True Damage"),
	Heal UMETA(DisplayName = "Heal"),
};

UENUM(BlueprintType)
enum class EWeaponDamageType : uint8
{
	None			UMETA(DisplayName = "No weapon damage"),
	MainHand		UMETA(DisplayName = "Ability only uses mainhand weapon damage"),
	Offhand			UMETA(DisplayName = "Ability only uses offhand weapon damage"),
	BothHands		UMETA(DisplayName = "Ability Hits with both weapons"),
};

UCLASS(Blueprintable)
class MOBA_API UMOBAAbilityData : public UObject 
{
	GENERATED_BODY()

public:
	// What type of damage does the ability do, if any?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOBA Ability Data")
		EMOBADamageType DamageType = EMOBADamageType::None;
	// What type of weapon damage does the ability do, if any?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOBA Ability Data")
		EWeaponDamageType WeaponDamageType = EWeaponDamageType::None;
	// Attack Power Ratio to apply to calculation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOBA Ability Data")
		float AttackPowerRatio = 0.0f;
	// Spell Power Ratio to apply to calculation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOBA Ability Data")
		float SpellPowerRatio = 0.0f;
	// Max Health Ratio to apply to calculation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOBA Ability Data")
		float MaxHealthRatio = 0.0f;
	// Missing Health Ratio to apply to calculation
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOBA Ability Data")
		float MissingHealthRatio = 0.0f;
	// Base Damage/Healing of the spell before adding ratios
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOBA Ability Data")
		float BaseValue = 0.0f;
};

UCLASS(Blueprintable)
class MOBA_API UMOBAGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	// Whether the ability should always be active.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOBA Ability Properties")
		bool bPassiveAbility = false;
	// Class to define ability information.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOBA Ability Properties")
		TSubclassOf<UMOBAAbilityData> AbilityData;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MOBA Ability Properties")
		float AbilityRange = 150.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MOBA Ability Components")
		USphereComponent* RangeSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MOBA Ability Properties")
		AMOBACharacter* MyCharacter;

	UFUNCTION(BlueprintCallable)
		FORCEINLINE TSubclassOf<UMOBAAbilityData> GetAbilityData() const {return AbilityData;	}
	
	UFUNCTION(BlueprintCallable)
		bool InRangeForAbility(FVector TargetLocation, AMOBACharacter* TargetCharacter = NULL);

	/** Called when the ability is given to an AbilitySystemComponent */
	virtual void OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	void OnAbilityEnded(UGameplayAbility* InAbility);

protected:
	virtual void PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate) override;
};
