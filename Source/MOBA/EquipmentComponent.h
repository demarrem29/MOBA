// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayAbilities/Public/Abilities/GameplayAbility.h"
#include "GameplayAbilities/Public/GameplayEffect.h"
#include "Image.h"
#include "UObject/Class.h"
#include "Projectile.h"
#include "EquipmentComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryChange);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEquipmentChange);

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
	Armor UMETA(DisplayName = "ArmorSlot"),
	BrainImplant UMETA(DisplayName = "BrainImplantSlot"),
	BodyImplant UMETA(DisplayName = "BodyImplantSlot"),
	MainHand UMETA(DisplayName = "MainHandSlot"),
	OffHand UMETA(DisplayName = "OffHandSlot"),
};

// Enum defining messages from equipping items
UENUM(BlueprintType)
enum class EInventoryMessage : uint8
{
	Success UMETA(DisplayName = "Success"),
	InventoryFull UMETA(DisplayName = "InventoryFull"),
	Unique UMETA(DisplayName = "Unique"),
	DoesNotExist UMETA(DisplayName = "DoesNotExist"),
	ModuleSlotsFull UMETA(DisplayName = "ModuleSlotsFull"),
	WrongSlot UMETA(DisplayName = "WrongEquipmentSlot"),
	InvalidEquipment UMETA(DisplayName = "InvalidEquipment"),
};

// Base characteristics that all items have
UCLASS(Blueprintable, BlueprintType)
class UItem : public UObject
{
	GENERATED_BODY()
	
	
protected:
	// Character that owns this item
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "ItemProperties")
	AMOBACharacter* MyOwner; 
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ItemProperties")
	EItemType ItemType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	FName ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	UTexture2D* IconImageSource;

	// How many items currently occupy one inventory slot
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "InventoryProperties")
	int32 CurrentStacks; 
	
	// How many items can occupy one inventory slot
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryProperties")
	int32 MaxStacks; 

	// More than one allowed per inventory?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InventoryProperties")
	bool bUniqueOwned; 

	// Abilities and/or stats granted by the equipped item. Effect and effect level
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Granted Effects")
		TMap<TSubclassOf<class UGameplayEffect>, float> GrantedEffects;

public:
	void SetCurrentStacks(int32 NewStackCount);
	FORCEINLINE void SetOwner(AMOBACharacter* NewOwner) { MyOwner = NewOwner; }
	FORCEINLINE AMOBACharacter* GetOwner() const { return MyOwner; }
	FORCEINLINE EItemType GetItemType() const { return ItemType; }
	FORCEINLINE FName GetItemName() const { return ItemName; }
	FORCEINLINE bool GetUniqueOwned() const { return bUniqueOwned; }
	FORCEINLINE int32 GetCurrentStacks() const { return CurrentStacks; }
	FORCEINLINE int32 GetMaxStacks() const { return MaxStacks; }
	FORCEINLINE TMap<TSubclassOf<class UGameplayEffect>, float> GetGrantedEffects() const { return GrantedEffects; }
};

UCLASS(Blueprintable, BlueprintType)
class UEquipment : public UItem
{
	GENERATED_BODY()
protected:	
	// Only one allowed per character?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EquipmentProperties")
		bool bUniqueEquipped; 

	// Only one allowed per item?
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modules")
		bool bModuleUniqueEquipped; 

	// How many modules are allowed to be equipped
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Modules")
		int32 MaxModuleSlots; 

	// Array of currently equipped modules
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Modules")
		TArray<UEquipment*> EquippedModules;

public:
	FORCEINLINE bool GetUniqueEquipped() { return bUniqueEquipped; }
	FORCEINLINE bool GetModuleUniqueEquipped() { return bModuleUniqueEquipped; }
	FORCEINLINE int32 GetMaxSlots() { return MaxModuleSlots; }
	FORCEINLINE TArray<UEquipment*> GetEquippedModules() { return EquippedModules; }
};

// Weapon-specific characteristics like damage, attack speed, etc.
UCLASS(Blueprintable, BlueprintType)
class UWeapon : public UEquipment 
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
	FORCEINLINE bool GetUseProjectile() { return bUseProjectile; }
	FORCEINLINE UClass* GetProjectileClass() { return Cast<UClass>(ProjectileClass); }
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MOBA_API UEquipmentComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UEquipmentComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TArray<UItem*> Inventory; // Items the character currently owns

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	TArray<UItem*> RemovedInventory; // Probably will move this to an independent shop later

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Inventory")
	int32 MaxInventorySize;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "EquipmentSlots")
	TMap<ESlotType, UEquipment*> EquipmentSlots;

	UFUNCTION(BlueprintCallable)
	void AddItemToInventory(const TSubclassOf<class UItem> ItemClass, TArray<UItem*> &ReturnedItems, EInventoryMessage &Message, UItem* const ExistingItem = NULL, const int32 Quantity = 1);

	UFUNCTION(BlueprintCallable)
	EInventoryMessage RemoveItemFromInventory(UItem* ItemToRemove, bool Delete = false, int32 Quantity = 1);

	UFUNCTION(BlueprintCallable)
	EInventoryMessage Equip(ESlotType SlotToEquip, UEquipment* ItemToEquip);
	
	UFUNCTION(BlueprintCallable)
	EInventoryMessage UnEquip(ESlotType SlotToUnequip);

// Helper Inventory functions, not to be called directly
private:
	UFUNCTION()
		bool ClassAlreadyPresentInInventory(TSubclassOf<UItem> ItemClass);
	UFUNCTION()
		TArray<UItem*> ItemInstancesAlreadyPresentInInventory(TSubclassOf<UItem> ItemClass);
	UFUNCTION()
		bool ItemInstanceAlreadyPresentInInventory(UItem* Item);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;	
	FOnInventoryChange OnInventoryChange;
	FOnEquipmentChange OnEquipmentChange;
};
