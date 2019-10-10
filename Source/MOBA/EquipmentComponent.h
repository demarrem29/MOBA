// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayAbilities/Public/Abilities/GameplayAbility.h"
#include "GameplayAbilities/Public/GameplayEffect.h"
#include "Image.h"
#include "EquipmentComponent.generated.h"

// Enum defining types of inventory slots
UENUM(BlueprintType)
enum class EItemType : uint8 
{
	Consumable UMETA(DisplayName = "Consumable"),
	Armor UMETA(DisplayName = "Armor"),
	BrainImplant UMETA(DisplayName = "BrainImplant"),
	BodyImplant UMETA(DisplayName = "BodyImplant"),
	MainHandWeapon UMETA(DisplayName = "MainHandWeapon"),
	OffHandWeapon UMETA(DisplayName = "OffHandWeapon"),
	OffHandSource UMETA(DisplayName = "OffHandSource"),
	TwoHandWeapon UMETA(DisplayName = "TwoHandWeapon"),
	ArmorModule UMETA(DisplayName = "ArmorModule"),
	WeaponModule UMETA(DisplayName = "WeaponModule")
};

// Base characteristics that all items have
USTRUCT(BlueprintType)
struct FItem
{
	GENERATED_BODY();
	
	FItem();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemProperties")
	EItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemProperties")
	FName ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemProperties")
	UImage* ItemIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemProperties")
	int32 NumModuleSlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Granted Abilities")
	TArray<UGameplayAbility*> GrantedAbilities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Granted Effects")
	TArray<UGameplayEffect*> GrantedEffects;
};

// Weapon-specific characteristics like damage, attack speed, etc.
USTRUCT(BlueprintType)
struct FWeapon : public FItem 
{
	GENERATED_BODY();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemProperties")
	int32 MinDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemProperties")
	int32 MaxDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemProperties")
	float AttackSpeed;
};


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MOBA_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEquipmentComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventorySlots")
	TMap<EItemType, FItem> InventorySlots;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
