// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "GameplayAbilitySpec.h"
#include "Components/SphereComponent.h"
#include "EquipmentComponent.h"
#include "Animation/AnimMontage.h"
#include "MOBACharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombatStatusChange, bool, bIsAttacking, bool, bIsInCombat);

UENUM(BlueprintType)
enum class ETeam : uint8
{
	BottomSide		UMETA(DisplayName = "Bottom Side"),
	TopSide			UMETA(DisplayName = "Top Side"),
	NeutralHostile	UMETA(DisplayName = "Jungle Camps"),
	NeutralFriendly UMETA(DisplayName = "Shop Vendors"),

};

UENUM(BlueprintType)
enum class AbilityInput : uint8
{
	UseAbility1 UMETA(DisplayName = "Use Spell 1"), //This maps the first ability(input ID should be 0 in int) to the action mapping(which you define in the project settings) by the name of "UseAbility1". "Use Spell 1" is the blueprint name of the element.
	UseAbility2 UMETA(DisplayName = "Use Spell 2"), //Maps ability 2(input ID 1) to action mapping UseAbility2. "Use Spell 2" is mostly used for when the enum is a blueprint variable.
	UseAbility3 UMETA(DisplayName = "Use Spell 3"),
	UseAbility4 UMETA(DisplayName = "Use Spell 4"),
	WeaponAbility UMETA(DisplayName = "Use Weapon"), //This finally maps the fifth ability(here designated to be your weaponability, or auto-attack, or whatever) to action mapping "WeaponAbility".

		//You may also do something like define an enum element name that is not actually mapped to an input, for example if you have a passive ability that isn't supposed to have an input. This isn't usually necessary though as you usually grant abilities via input ID,
		//which can be negative while enums cannot. In fact, a constant called "INDEX_NONE" exists for the exact purpose of rendering an input as unavailable, and it's simply defined as -1.
		//Because abilities are granted by input ID, which is an int, you may use enum elements to describe the ID anyway however, because enums are fancily dressed up ints.
};

UCLASS(Blueprintable)
class AMOBACharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AMOBACharacter();

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Ability System")
	class UAbilitySystemComponent* AbilitySystemComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Ability System")
		class UMOBAAttributeSet* AttributeSet;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Equipment")
		class UEquipmentComponent* EquipmentComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
		ETeam MyTeam;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
		int32 PlayerIndex;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
		bool bIsAttacking;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
		bool bIsInCombat;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BasicAttack")
		int32 ComboIndex = 0;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BasicAttack")
		bool bUseOffHandWeapon = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BasicAttack")
		TArray<UAnimMontage*> BasicAttackAnimations;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BasicAttack")
		bool bUseProjectile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BasicAttack")
		TSubclassOf<AProjectile> ProjectileClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BasicAttack")
		FName ProjectileSpawnSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
		AMOBACharacter* MyEnemyTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
		AMOBACharacter* MyFollowTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
		AMOBACharacter* MyFocusTarget;

	UFUNCTION(BlueprintCallable, Category = "Combat")
		bool IsHostile(AMOBACharacter* TargetCharacter);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ProjectileTarget") // Scene component for homing projectiles to target.
		USceneComponent* ProjectileTarget;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Abilities")
		void AcquireAbility(TSubclassOf<UGameplayAbility> AbilityToAcquire);

	UFUNCTION(BlueprintCallable, Category = "Abilities")
		void RemoveAbility(TSubclassOf<UGameplayAbility> AbilityToRemove);

	UFUNCTION(BlueprintCallable, Category = "Abilities")
		float GetBasicAttackCooldown();

	UFUNCTION(BlueprintCallable, Category = "Equipment")
		bool GetOffHandWeaponEquipped();

	FTimerHandle CombatTimerHandle;	

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;

	// Event Handlers for receiving attribute set delegate broadcasts
	UFUNCTION()
		void InventoryChange(TArray<UItem*> AffectedSlots, TArray<int32> AffectedIndices);
	UFUNCTION()
		void EquipmentChange(uint8 AffectedSlot, UEquipment* EquipmentObjRef);
	UFUNCTION()
		void HealthChange(FGameplayAttributeData health, FGameplayAttributeData maxhealth);
	UFUNCTION()
		void HealthRegenChange(FGameplayAttributeData HealthRegen);
	UFUNCTION()
		void HealingModifierChange(FGameplayAttributeData HealingModifier);
	UFUNCTION()
		void ManaChange(FGameplayAttributeData mana, FGameplayAttributeData maxmana);
	UFUNCTION()
		void ManaRegenChange(FGameplayAttributeData ManaRegen);
	UFUNCTION()
		void LevelChange(FGameplayAttributeData level, FGameplayAttributeData maxlevel);
	UFUNCTION()
		void ExperienceChange(FGameplayAttributeData experience, FGameplayAttributeData maxexperience);
	UFUNCTION()
		void AttackPowerChange(FGameplayAttributeData AttackPower);
	UFUNCTION()
		void SpellPowerChange(FGameplayAttributeData SpellPower);
	UFUNCTION()
		void MainHandChange(FGameplayAttributeData MainHandAttackSpeed, FGameplayAttributeData MainHandMinDamage, FGameplayAttributeData MainHandMaxDamage, FGameplayAttributeData MainHandAttackRange);
	UFUNCTION()
		void OffHandChange(FGameplayAttributeData OffHandAttackSpeed, FGameplayAttributeData OffHandMinDamage, FGameplayAttributeData OffHandMaxDamage, FGameplayAttributeData OffHandAttackRange);
	UFUNCTION()
		void BonusAttackSpeedChange(FGameplayAttributeData BonusttackSpeed);
	UFUNCTION()
		void CriticalChanceChange(FGameplayAttributeData CriticalChance);
	UFUNCTION()
		void CriticalDamageChange(FGameplayAttributeData CriticalDamage);
	UFUNCTION()
		void ArmorChange(FGameplayAttributeData Armor);
	UFUNCTION()
		void PhysicalDamageReductionChange(FGameplayAttributeData PhysicalDamageReduction);
	UFUNCTION()
		void EnvironmentalResistanceChange(FGameplayAttributeData EnvironmentalResistance);
	UFUNCTION()
		void EnvironmentalDamageReductionChange(FGameplayAttributeData EnvironmentalDamageReduction);
	UFUNCTION()
		void FlatDamageReductionChange(FGameplayAttributeData FlatDamageReduction);
	UFUNCTION()
		void MovementSpeedChange(FGameplayAttributeData MovementSpeed);
	UFUNCTION()
		void CombatStatusChange(bool bIsAttackingIn, bool bIsInCombatIn);
	UFUNCTION()
		void OnAbilityEnded(const FAbilityEndedData& AbilityEndData);
	UFUNCTION()
		void OnGameplayEffectEnd(const FActiveGameplayEffect& EndedGameplayEffect);
	UFUNCTION()
		void TryBasicAttack();

	// Called by the above event handlers to expose to blueprints. Useful for updating UI.
	UFUNCTION(BlueprintImplementableEvent)
		void BP_InventoryChange(const TArray<UItem*>& AffectedSlots, const TArray<int32>& AffectedIndices);
	UFUNCTION(BlueprintImplementableEvent)
		void BP_EquipmentChange(ESlotType AffectedSlot, UEquipment* EquipmentObjRef);
	UFUNCTION(BlueprintImplementableEvent)
		void BP_HealthChange(FGameplayAttributeData health, FGameplayAttributeData maxhealth);
	UFUNCTION(BlueprintImplementableEvent)
		void BP_HealthRegenChange(FGameplayAttributeData HealthRegen);
	UFUNCTION(BlueprintImplementableEvent)
		void BP_HealingModifierChange(FGameplayAttributeData HealingModifier);
	UFUNCTION(BlueprintImplementableEvent)
		void BP_ManaChange(FGameplayAttributeData mana, FGameplayAttributeData maxmana);
	UFUNCTION(BlueprintImplementableEvent)
		void BP_ManaRegenChange(FGameplayAttributeData ManaRegen);
	UFUNCTION(BlueprintImplementableEvent)
		void BP_LevelChange(FGameplayAttributeData level, FGameplayAttributeData maxlevel);
	UFUNCTION(BlueprintImplementableEvent)
		void BP_ExperienceChange(FGameplayAttributeData experience, FGameplayAttributeData maxexperience);
	UFUNCTION(BlueprintImplementableEvent)
		void BP_AttackPowerChange(FGameplayAttributeData AttackPower);
	UFUNCTION(BlueprintImplementableEvent)
		void BP_SpellPowerChange(FGameplayAttributeData SpellPower);
	UFUNCTION(BlueprintImplementableEvent)
		void BP_MainHandChange(FGameplayAttributeData MainHandAttackSpeed, FGameplayAttributeData MainHandMinDamage, FGameplayAttributeData MainHandMaxDamage, FGameplayAttributeData MainHandAttackRange);
	UFUNCTION(BlueprintImplementableEvent)
		void BP_OffHandChange(FGameplayAttributeData OffHandAttackSpeed, FGameplayAttributeData OffHandMinDamage, FGameplayAttributeData OffHandMaxDamage, FGameplayAttributeData OffHandAttackRange);
	UFUNCTION(BlueprintImplementableEvent)
		void BP_BonusAttackSpeedChange(FGameplayAttributeData BonusAttackSpeed);
	UFUNCTION(BlueprintImplementableEvent)
		void BP_CriticalChanceChange(FGameplayAttributeData CriticalChance);
	UFUNCTION(BlueprintImplementableEvent)
		void BP_CriticalDamageChange(FGameplayAttributeData CriticalDamage);
	UFUNCTION(BlueprintImplementableEvent)
		void BP_ArmorChange(FGameplayAttributeData Armor);
	UFUNCTION(BlueprintImplementableEvent)
		void BP_PhysicalDamageReductionChange(FGameplayAttributeData PhysicalDamageReduction);
	UFUNCTION(BlueprintImplementableEvent)
		void BP_EnvironmentalResistanceChange(FGameplayAttributeData Armor);
	UFUNCTION(BlueprintImplementableEvent)
		void BP_EnvironmentalDamageReductionChange(FGameplayAttributeData EnvironmentalDamageReduction);
	UFUNCTION(BlueprintImplementableEvent)
		void BP_FlatDamageReductionChange(FGameplayAttributeData FlatDamageReduction);
	UFUNCTION(BlueprintImplementableEvent)
		void BP_MovementSpeedChange(FGameplayAttributeData MovementSpeed);
	UFUNCTION(BlueprintImplementableEvent)
		void BP_CombatStatusChange(bool bIsAttackingIn, bool bIsInCombatIn);
	UFUNCTION(BlueprintImplementableEvent)
		void BP_OnAbilityEnded(const FAbilityEndedData& AbilityEndData);
	UFUNCTION(BlueprintImplementableEvent)
		void BP_OnGameplayEffectEnd(const FActiveGameplayEffect& EndedGameplayEffect);
	UFUNCTION(BlueprintImplementableEvent)
		void BP_TryBasicAttack(bool UseOffHand);
	
	FCombatStatusChange CombatStatusChangeDelegate;

protected:
	void CombatTimerCallback();
};

