// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentComponent.h"
#include "MOBACharacter.h"
#include "MOBAAttributeSet.h"
#include "AbilitySystemComponent.h"

void UItem::SetCurrentStacks(int32 NewStackCount)
{
	// Destroy item if new stack count is 0
	if (NewStackCount == 0)
	{
		MyOwner->EquipmentComponent->RemoveItemFromInventory(this, true);
		return;
	}

	// Stack count was not zero, use new value
	CurrentStacks = NewStackCount;
	FMath::Clamp(CurrentStacks, 1, MaxStacks); // Enforce valid stack count
}

// Sets default values for this component's properties
UEquipmentComponent::UEquipmentComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	MaxInventorySize = 6;
}


// Called when the game starts
void UEquipmentComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UEquipmentComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

// Function to add an item to inventory. ExistingItem is an optional parameter. WARNING: ReturnedItem may be NULL.
void UEquipmentComponent::AddItemToInventory(const TSubclassOf<class UItem> ItemClass, TArray<UItem*> &ReturnedItems, EInventoryMessage &Message, UItem* const ExistingItem, const int32 Quantity)
{
	bool Instanced = false;
	bool AnotherExists = false;
	bool Unique = false;
	bool CanStack = false;
	int32 AvailableInventorySlots;
	UItem* Item;
	int32 QuantityRemaining = Quantity;
	int32 InventorySlotsRequired;

	UObject* ItemBPObj = ItemClass->ClassDefaultObject;
	Item = Cast<UItem>(ItemBPObj);
	// Verify item class is valid, abort if not
	if (!Item) 
	{
		Message = EInventoryMessage::DoesNotExist;
		return;
	}
	
	AnotherExists = ClassAlreadyPresentInInventory(ItemClass); // Whether another item of the same class is present in inventory
	Unique = Item->GetUniqueOwned();			// Multiple of the same item class allowed in inventory?

	// Abort if the item is unique and another is in inventory
	if (AnotherExists && Unique)
	{
		Message = EInventoryMessage::Unique;
		return;
	}

	InventorySlotsRequired = (Quantity + Item->GetMaxStacks() - 1) / Item->GetMaxStacks(); // How many inventory slots are required
	AvailableInventorySlots = MaxInventorySize - Inventory.Num();

	int32 SlotsUsed;
	
	// Should we try to add stacks to an existing inventory slot before filling a new inventory slot?
	CanStack = (Item->GetMaxStacks() > 1) ? true : false;

	// If we can stack, check and see if any of the existing items have room for more stacks
	TArray<UItem*> FilterArray = ItemInstancesAlreadyPresentInInventory(ItemClass);
	if (CanStack) 
	{
		// Get all items in inventory of the same class and check and see if any of them have room for more stacks
		SlotsUsed = InventorySlotsRequired;
		int32 QuantityStacked = 0;
		
		for (auto& AnotherItem : FilterArray)
		{
			// Test if the operation can be performed prior to actually changing anything
			if (AnotherItem->GetCurrentStacks() < AnotherItem->GetMaxStacks())
			{
				// Item with available stacks found, add stacks and decrement number of stacks to add
				int32 AvailableStacks = AnotherItem->GetMaxStacks() - AnotherItem->GetCurrentStacks();
				int32 StacksUsed = (QuantityRemaining >= AvailableStacks) ? AvailableStacks : QuantityRemaining;
				QuantityRemaining -= StacksUsed;
				SlotsUsed--;
				if (QuantityRemaining <= 0) break;
			}
		}
		if (SlotsUsed > AvailableInventorySlots) 
		{
			Message = EInventoryMessage::InventoryFull;
			return;
		}
	}
	else // Item can't stack, each quantity is its own inventory slot
	{
		SlotsUsed = Quantity;
		if (SlotsUsed > AvailableInventorySlots) 
		{
			Message = EInventoryMessage::InventoryFull;
			return;
		}
	}

	// Ready to actually make inventory changes, reset remaining quantity
	QuantityRemaining = Quantity;

	// Use stacks of existing item instances first
	if (CanStack) 
	{
		for (auto& AnotherItem : FilterArray)
		{
			UItem* CurrentItem = CastChecked<UItem>(AnotherItem);
			// Find other item instances with available stacks
			if (CurrentItem->GetCurrentStacks() < CurrentItem->GetMaxStacks())
			{
				// Item with available stacks found, add stacks and decrement number of stacks to add
				int32 AvailableStacks = CurrentItem->GetMaxStacks() - CurrentItem->GetCurrentStacks();
				// Use whichever value is lower, quantity remaining or available stacks
				int32 StacksUsed = (QuantityRemaining >= AvailableStacks) ? AvailableStacks : QuantityRemaining;
				QuantityRemaining -= StacksUsed;
				
				// Update item to reflect stack change
				CurrentItem->SetCurrentStacks(CurrentItem->GetCurrentStacks() + StacksUsed);
				// End if there are no more stacks to add
				if (QuantityRemaining <= 0) break;
			}
		}
	}
	
	// Create new inventory slots for each remaining slot required
	for (int32 i = 0; i < SlotsUsed; i++) 
	{
		if (i == 0) 
		{
			// Get Item Properties to determine what we can do
			Instanced = Cast<UItem>(ExistingItem) ? true : false; // Does the item exist already or do we need to create a new instance
			if (!Instanced)
			{
				Item = NewObject<UItem>(this, ItemClass);
			}
			else 
			{
				Item = ExistingItem;
			}
		}
		else 
		{
			Item = NewObject<UItem>(this, ItemClass);
		}
		int32 AvailableStacks = Item->GetMaxStacks();
		int32 StacksUsed = (QuantityRemaining >= AvailableStacks) ? AvailableStacks : QuantityRemaining;
		QuantityRemaining -= StacksUsed;
		Item->SetCurrentStacks(StacksUsed);
		Item->SetOwner(CastChecked<AMOBACharacter>(this->GetOwner()));
		Inventory.Add(Item);
		ReturnedItems.Add(Item);
		if (QuantityRemaining <= 0) break;
	}
	Message = EInventoryMessage::Success;
	return;	
}

// Function to remove an item from inventory. 
EInventoryMessage UEquipmentComponent::RemoveItemFromInventory(UItem* ItemToRemove, bool Delete, int32 Quantity)
{
	if (Inventory.Contains(ItemToRemove)) 
	{
		Inventory.Remove(ItemToRemove);
		if (Delete) 
		{
			ItemToRemove->BeginDestroy();
		}
		else 
		{
			RemovedInventory.Add(ItemToRemove);
		}
		return EInventoryMessage::Success;
	}
	else return EInventoryMessage::DoesNotExist;
}



bool UEquipmentComponent::ClassAlreadyPresentInInventory(TSubclassOf<UItem> ItemClass)
{
	if (Inventory.ContainsByPredicate([&](const UObject* Object) {return Object->GetClass() == ItemClass; }))
	{
		// Another copy exists
		return true;
	}
	return false;
}

TArray<UItem*> UEquipmentComponent::ItemInstancesAlreadyPresentInInventory(TSubclassOf<UItem> ItemClass) 
{
	auto Filter = Inventory.FilterByPredicate([&](const UObject* Object) {return Object->GetClass() == ItemClass; });
	return Filter;
}

bool UEquipmentComponent::ItemInstanceAlreadyPresentInInventory(UItem* Item) 
{
	
	TArray<UItem*> PresentItemsOfSameClass = ItemInstancesAlreadyPresentInInventory(Item->GetClass());
	for (auto& ItemInstance : PresentItemsOfSameClass) 
	{
		if (ItemInstance == Item) return true;
	}
	return false;
}


// Function to equip a new item on the character.
EInventoryMessage UEquipmentComponent::Equip(ESlotType SlotToEquip, UEquipment* ItemToEquip)
{
	// Make sure that passed in item is a valid equippable item
	if (ItemToEquip) 
	{
		AMOBACharacter* MyOwner = Cast<AMOBACharacter>(this->GetOwner());
		UItem* UpcastItem = CastChecked<UItem>(ItemToEquip);
		if (MyOwner) 
		{
			// Verify that the item can actually be equipped in the desired slot
			EItemType ItemType = UpcastItem->GetItemType();
			
			switch (ItemType) 
			{
			case EItemType::Armor: if (SlotToEquip != ESlotType::Armor) return EInventoryMessage::WrongSlot;
				break;
			case EItemType::ArmorModule: if (SlotToEquip != ESlotType::Armor) return EInventoryMessage::WrongSlot;
				break;
			case EItemType::BrainImplant: if (SlotToEquip != ESlotType::BrainImplant) return EInventoryMessage::WrongSlot;
				break;
			case EItemType::BodyImplant: if (SlotToEquip != ESlotType::BodyImplant) return EInventoryMessage::WrongSlot;
				break;
			case EItemType::OneHand:if ((SlotToEquip != ESlotType::MainHand) || (SlotToEquip != ESlotType::OffHand)) return EInventoryMessage::WrongSlot;
				break;
			case EItemType::TwoHand:if (SlotToEquip != ESlotType::MainHand) return EInventoryMessage::WrongSlot;
				break;
			case EItemType::Source:if (SlotToEquip != ESlotType::OffHand) return EInventoryMessage::WrongSlot;
				break;
			case EItemType::WeaponModule:if ((SlotToEquip != ESlotType::MainHand) || (SlotToEquip != ESlotType::OffHand)) return EInventoryMessage::WrongSlot;
				break;
			default: return EInventoryMessage::InvalidEquipment;
			}

			// Check if an item is already equipped in the slot
			// Note: if the item is a module, there must already be an equipped item in the slot
			if (EquipmentSlots.Contains(SlotToEquip)) 
			{
				if (ItemType == EItemType::ArmorModule || ItemType == EItemType::WeaponModule)
				{
					// Verify modules are placed on the correct type of equipment
					if (ItemType == EItemType::ArmorModule && SlotToEquip != ESlotType::Armor) return EInventoryMessage::WrongSlot;
					UEquipment* FoundItem = *EquipmentSlots.Find(SlotToEquip);
					if (ItemType == EItemType::WeaponModule && Cast<UWeapon>(FoundItem) == NULL) return EInventoryMessage::WrongSlot;

					 UEquipment* FoundEquipment = *EquipmentSlots.Find(SlotToEquip);
					TArray<UEquipment*> ExistingModules = FoundEquipment->GetEquippedModules();
					// Check if there is room for another module
					if ((FoundEquipment->GetMaxSlots() > ExistingModules.Num()) && (ExistingModules.Num()  >= 0))
					{
						ExistingModules.Add(ItemToEquip);
						if (ItemInstanceAlreadyPresentInInventory(ItemToEquip)) 
						{
							RemoveItemFromInventory(ItemToEquip);
						}
					}
					else return EInventoryMessage::ModuleSlotsFull;

				}
				else // Not a module, need to unequip existing item
				{
					if (ItemToEquip->GetItemType() == EItemType::TwoHand) 
					{
						bool TwoHandRemovedFromInventory = false;
						// Make sure that we actually can put two weapons back in inventory to equip a two hand weapon
						if (Inventory.Contains(ItemToEquip)) 
						{ 
							Inventory.Remove(ItemToEquip);
							TwoHandRemovedFromInventory = true;
						}
						// Check if there are enough inventory slots to do the transaction
						if (Inventory.Num() + 2 > MaxInventorySize) 
						{
							if (TwoHandRemovedFromInventory) Inventory.Add(ItemToEquip);
							return EInventoryMessage::InventoryFull;
						}
					}
					// There is, try to unequip it

					// Two hand needs to unequip both slots 
					if (ItemToEquip->GetItemType() == EItemType::TwoHand) 
					{
						UnEquip(ESlotType::MainHand);
						UnEquip(ESlotType::OffHand);
					}
					else
					{
						EInventoryMessage UnEquip = this->UnEquip(SlotToEquip);
						if (UnEquip != EInventoryMessage::Success) 
						{
							return UnEquip; // Item could not be unequipped. Maybe not enough inventory space
						}
						
					}
				}	
			}
			else // Did not contain an equipped item already
			{
				if (ItemType == EItemType::ArmorModule || ItemType == EItemType::WeaponModule) 
				{
					return EInventoryMessage::InvalidEquipment; // Can't equip a module on an empty slot
				}
			}
			// Equip new item if not a module (module already equipped above)
			if (ItemType != EItemType::ArmorModule && ItemType != EItemType::WeaponModule) 
			{
				EquipmentSlots.Add(SlotToEquip, ItemToEquip); // Add the item to the equipment slots TMAP
			}
			Cast<UItem>(ItemToEquip)->SetOwner(MyOwner); // Add the item owner
			// Apply Gameplay Effects to Owner
			for (auto Effect : Cast<UEquipment>(ItemToEquip)->GetGrantedEffects())
			{
				FGameplayEffectContextHandle Context = MyOwner->AbilitySystemComponent->MakeEffectContext();
				MyOwner->AbilitySystemComponent->BP_ApplyGameplayEffectToSelf(Effect.Key, Effect.Value, Context);
			}
			if (ItemType == EItemType::OneHand || ItemType == EItemType::TwoHand) // Broadcast weapon attribute changes. Other attribute changes handled by attribute set
			{
				if (SlotToEquip == ESlotType::MainHand) MyOwner->AttributeSet->MainHandChange.Broadcast(MyOwner->AttributeSet->MainHandAttackSpeed.GetCurrentValue(), MyOwner->AttributeSet->MainHandMinDamage.GetCurrentValue(), MyOwner->AttributeSet->MainHandMaxDamage.GetCurrentValue(), MyOwner->AttributeSet->MainHandAttackRange.GetCurrentValue());
				else if (SlotToEquip == ESlotType::OffHand) MyOwner->AttributeSet->OffHandChange.Broadcast(MyOwner->AttributeSet->OffHandAttackSpeed.GetCurrentValue(), MyOwner->AttributeSet->OffHandMinDamage.GetCurrentValue(), MyOwner->AttributeSet->OffHandMaxDamage.GetCurrentValue(), MyOwner->AttributeSet->OffHandAttackRange.GetCurrentValue());
			}
			return EInventoryMessage::Success;
			
		}
		return EInventoryMessage::DoesNotExist;
	}
	return EInventoryMessage::DoesNotExist;
}

EInventoryMessage UEquipmentComponent::UnEquip(ESlotType SlotToUnequip)
{
	UEquipment* ItemToUnequip = *EquipmentSlots.Find(SlotToUnequip);
	// Check if there is a valid item in the slot
	if (!ItemToUnequip) return EInventoryMessage::DoesNotExist;
	// Check if there is room to move the item from equipped to inventory
	if (Inventory.Num() >= MaxInventorySize) return EInventoryMessage::InventoryFull;
	// Checks passed, remove from equipment slots and add to inventory
	EquipmentSlots.Remove(SlotToUnequip);
	//Remove Effects granted by the item
	for (auto Effect : Cast<UEquipment>(ItemToUnequip)->GetGrantedEffects())
	{
		FGameplayEffectContextHandle Context = ItemToUnequip->GetOwner()->AbilitySystemComponent->MakeEffectContext();
		ItemToUnequip->GetOwner()->AbilitySystemComponent->RemoveActiveGameplayEffectBySourceEffect(Effect.Key, ItemToUnequip->GetOwner()->AbilitySystemComponent);
	}
	Inventory.Add(ItemToUnequip);
	return EInventoryMessage::Success;
}

