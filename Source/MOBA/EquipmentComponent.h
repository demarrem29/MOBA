// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayAbilities/Public/Abilities/GameplayAbility.h"
#include "GameplayAbilities/Public/GameplayEffect.h"
#include "Image.h"
#include "EquipmentComponent.generated.h"


class AMOBACharacter;
// Enum defining types of items
UENUM(BlueprintType)
enum class EItemType : uint8 
{
	Consumable UMETA(DisplayName = "Consumable"),
	Armor UMETA(DisplayName = "Armor"),
	BrainImplant UMETA(DisplayName = "BrainImplant"),
	BodyImplant UMETA(DisplayName = "BodyImplant"),
	OneHand UMETA(DisplayName = "OneHandWeapon"),
	TwoHand UMETA(DisplayName = "TwoHandWeapon"),
	Source UMETA(DisplayName = "Source"),
	ArmorModule UMETA(DisplayName = "ArmorModule"),
	WeaponModule UMETA(DisplayName = "WeaponModule")
};

// Enum defining types of equipment slots
UENUM(BlueprintType)
enum class ESlotType : uint8
{
	Consumable UMETA(DisplayName = "ConsumableSlot"),
	Armor UMETA(DisplayName = "ArmorSlot"),
	BrainImplant UMETA(DisplayName = "BrainImplantSlot"),
	BodyImplant UMETA(DisplayName = "BodyImplantSlot"),
	MainHand UMETA(DisplayName = "MainHandSlot"),
	OffHand UMETA(DisplayName = "OffHandSlot"),
};

// Base characteristics that all items have
UCLASS(Blueprintable, BlueprintType)
class UItem : public UObject
{
	GENERATED_BODY()
	
	
protected:
	UItem();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemProperties")
	AMOBACharacter* MyOwner;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemProperties")
	EItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemProperties")
	FName ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemProperties")
	UTexture2D* IconImageSource;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemProperties")
	UImage* IconImage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemProperties")
	int32 NumModuleSlots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Granted Abilities")
	TArray<TSubclassOf<class UGameplayAbility>> GrantedAbilities;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Granted Effects")
	TArray<TSubclassOf<class UGameplayEffect>> GrantedEffects;
public:
	bool Equip(AMOBACharacter* Owner);

	bool UnEquip();

	FORCEINLINE AMOBACharacter* GetOwner() { return MyOwner; }
	FORCEINLINE EItemType GetItemType() { return ItemType; }
	FORCEINLINE FName GetItemName() { return ItemName; }
	FORCEINLINE int32 GetNumSlots() { return NumModuleSlots; }
};

// Weapon-specific characteristics like damage, attack speed, etc.
UCLASS(Blueprintable, BlueprintType)
class UWeapon : public UItem 
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperties")
	int32 MinDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperties")
	int32 MaxDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperties")
	float AttackSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperties")
	float AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperties")
	bool bUseProjectile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "WeaponProperties")
	const TSubclassOf<class AProjectile> ProjectileClass;

public:
	FORCEINLINE int32 GetMinDamage() { return MinDamage; }
	FORCEINLINE int32 GetMaxDamage() { return MaxDamage; }
	FORCEINLINE float GetAttackSpeed() { return AttackSpeed; }
	FORCEINLINE float GetAttackRange() { return AttackRange; }
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MOBA_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEquipmentComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TArray<TSubclassOf<UItem>> Inventory;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EquipmentSlots")
	TMap<ESlotType, TSubclassOf<UItem>> EquipmentSlots;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;		
};
