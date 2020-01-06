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

ESlotType UEquipment::GetEquipmentSlotType() 
{
	switch (this->GetItemType()) 
	{
	case EItemType::Armor: return ESlotType::Armor;
	case EItemType::BrainImplant: return ESlotType::BrainImplant;
	case EItemType::BodyImplant: return ESlotType::BodyImplant;
	case EItemType::Source: return ESlotType::OffHand;
	case EItemType::TwoHand: return ESlotType::MainHand;
	case EItemType::ArmorModule: return ESlotType::Armor;
	case EItemType::WeaponModule: return ESlotType::EitherHand;
	case EItemType::MainHand: return ESlotType::MainHand;
	case EItemType::OneHand: return ESlotType::EitherHand;
	default: return ESlotType::None;
	}
}

// Sets default values for this component's properties
UEquipmentComponent::UEquipmentComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	// Set default MaxInventorySize and Initialize Inventory
	MaxInventorySize = 6;
	Inventory.Init(NULL, MaxInventorySize);
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
	// Initialize variables
	bool Instanced = false;
	bool AnotherExists = false;
	bool Unique = false;
	bool CanStack = false;
	int32 AvailableInventorySlots = 0;
	UItem* Item = NULL;
	int32 QuantityRemaining = Quantity;
	int32 InventorySlotsRequired = 0;
	int32 SlotsUsed = 0;
	TArray<int32> EmptyInventorySlotIndices; // Holds position of empty slots in inventory
	// Arrays for delegate to broadcast
	TArray<UItem*> AffectedItems;
	TArray<int32> AffectedIndices;
	// Use ItemClass to get a default object
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
	
	// Find number of available inventory slots
	for (int i = 0; i < MaxInventorySize; i++) 
	{
		// Create pointer to current inventory index
		UItem* CurrentItem = Inventory[i];
		// Check if a valid pointer is there
		if (!CurrentItem->IsValidLowLevel()) 
		{
			// No item is here, increment available slots and note the index of that empty slot
			AvailableInventorySlots++;
			EmptyInventorySlotIndices.Add(i);
		}
	}

	
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
				// Add this item to the list of affected items if we haven't already
				ReturnedItems.AddUnique(CurrentItem);
				AffectedItems.Add(CurrentItem);
				AffectedIndices.Add(Inventory.Find(CurrentItem));
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
		// Add item to inventory and update delegate arrays
		Inventory[EmptyInventorySlotIndices[i]] = Item;
		AffectedItems.Add(Item);
		AffectedIndices.Add(EmptyInventorySlotIndices[i]);
		// Add this item as a returned item reference
		ReturnedItems.Add(Item);
		if (QuantityRemaining <= 0) break;
	}
	Message = EInventoryMessage::Success;
	
	// Operation successfully added some items, broacast the delegate
	OnInventoryChange.Broadcast(AffectedItems, AffectedIndices);
	return;	
}

// Function to remove an item from inventory. 
EInventoryMessage UEquipmentComponent::RemoveItemFromInventory(UItem* ItemToRemove, bool Delete, int32 NumberOfStacksToRemove)
{
	// Verify that the item is actually in the inventory
	if (Inventory.Contains(ItemToRemove))
	{
		// Create return value for delegate and add the item as an affected inventory slot
		TArray<UItem*> AffectedItems;
		TArray<int32> AffectedIndices;
		
		AffectedIndices.Add(Inventory.Find(ItemToRemove));

		// Check if we are just removing some stacks or actually removing the entire item
		if (NumberOfStacksToRemove >= ItemToRemove->GetCurrentStacks()) 
		{	
			Inventory[Inventory.Find(ItemToRemove)] = NULL;
			if (ItemToRemove->GetItemType() == EItemType::Consumable || Delete) 
			{
				ItemToRemove->BeginDestroy();
			}
		}
		else 
		{
			// Only Removing Stacks, decrement stacks and add item to delegate. item is not added if removed/deleted
			ItemToRemove->SetCurrentStacks(ItemToRemove->GetCurrentStacks() - NumberOfStacksToRemove);
			AffectedItems.Add(ItemToRemove);
		}
				
		// Broadcast Delegate and return
		OnInventoryChange.Broadcast(AffectedItems, AffectedIndices);
		return EInventoryMessage::Success;
	}
	// Item did not exist, do nothing and return
	else return EInventoryMessage::DoesNotExist;
}

// Function that allows users to drag and drop items to move them around to preferred inventory location
UFUNCTION(BlueprintCallable)
EInventoryMessage UEquipmentComponent::SwapItemsInInventory(int32 Index1, int32 Index2) 
{
	// Verify that both indices are valid and that at least one of the items exists. We can move to an empty slot if one of the items doesn't exist
	if ((Inventory.IsValidIndex(Index1) && Inventory.IsValidIndex(Index2)) && (Inventory[Index1]->IsValidLowLevel() || Inventory[Index2]->IsValidLowLevel()))
	{
		// Create arrays for the delegate to broadcast
		TArray<UItem*> AffectedItems;
		TArray<int32> AffectedIndices;
		// Create item pointers
		UItem* FirstItem = Inventory[Index1];
		UItem* SecondItem = Inventory[Index2];
		// Swap items
		Inventory[Index1] = SecondItem;
		Inventory[Index2] = FirstItem;
		// Fill arrays for delegate to broadcast
		AffectedItems.Add(FirstItem);
		AffectedItems.Add(SecondItem);
		AffectedIndices.Add(Index2);
		AffectedIndices.Add(Index1);
		// Broadcast and return
		OnInventoryChange.Broadcast(AffectedItems, AffectedIndices);
		return EInventoryMessage::Success;
	}
	else return EInventoryMessage::DoesNotExist;
}

int32 UEquipmentComponent::GetEmptyInventorySlots(TArray<int32>& OptionalIndexArray) 
{
	int32 EmptySlotCount = 0;
	for (auto & Item : Inventory) 
	{
		if (!Item->IsValidLowLevel()) 
		{
			EmptySlotCount++;
			if (OptionalIndexArray.GetData() != nullptr) 
			{
				OptionalIndexArray.Add(Inventory.Find(Item));
			}
		}
	}
	return EmptySlotCount;
}

// This function determines if we should add new stacks to an existing item, or create a new one.
bool UEquipmentComponent::ClassAlreadyPresentInInventory(TSubclassOf<UItem> ItemClass)
{
	for (auto & Item : Inventory) 
	{
		if (Item->IsValidLowLevel()) 
		{
			if (Item->GetClass() == ItemClass) 
			{
				return true;
			}
		}
	}
	return false;
}

// Find all items in inventory of the given class
TArray<UItem*> UEquipmentComponent::ItemInstancesAlreadyPresentInInventory(TSubclassOf<UItem> ItemClass) 
{
	TArray<UItem*> Filter;
	for (auto & Item : Inventory) 
	{
		if (Item->IsValidLowLevel()) 
		{
			if (Item->GetClass() == ItemClass)
			{
				Filter.Add(Item);
			}
		}
	}
	
	return Filter;
}

// Are we adding stacks to a specific item?
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
				UEquipment* FoundEquipment = *EquipmentSlots.Find(SlotToEquip);
				// There is an item equipped already. Check if the item to equip is a module and if it can fit in SlotToEquip
				if (ItemType == EItemType::ArmorModule || ItemType == EItemType::WeaponModule)
				{
					// Verify modules are placed on the correct type of equipment
					if (ItemType == EItemType::ArmorModule && SlotToEquip != ESlotType::Armor) return EInventoryMessage::WrongSlot;
					
					if (ItemType == EItemType::WeaponModule && Cast<UWeapon>(FoundEquipment) == NULL) return EInventoryMessage::WrongSlot;

					// Check if there is room for another module
					TArray<UEquipment*> ExistingModules = FoundEquipment->GetEquippedModules();
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
					// Ignore two hand weapons for now, require additional checks that will be handled below
					if (ItemToEquip->GetItemType() != EItemType::TwoHand) 
					{
						EInventoryMessage UnEquip = this->UnEquip(SlotToEquip);
						if (UnEquip != EInventoryMessage::Success)
						{
							return UnEquip; // Item could not be unequipped. Maybe not enough inventory space
						}
					}
				}	
			}

			// Additional steps required for equipping a two hand weapon
			if (ItemToEquip->GetItemType() == EItemType::TwoHand)
			{
				int32 SlotsRequired = 0;
				bool TwoHandRemovedFromInventory = false;
				// Make sure that we actually can put enough items back in inventory to equip a two hand weapon
				if (ItemInstanceAlreadyPresentInInventory(ItemToEquip))
				{
					RemoveItemFromInventory(ItemToEquip);
					TwoHandRemovedFromInventory = true;
					SlotsRequired--;
				}
				// Check if there are enough inventory slots to do the transaction
				TArray<int32> EmptyInventorySlotsArray;
				TArray<UItem*> ReturnedItems;
				EInventoryMessage AddItemsMessage;
				
				if (EquipmentSlots.Contains(ESlotType::MainHand)) SlotsRequired++;
				if (EquipmentSlots.Contains(ESlotType::OffHand)) SlotsRequired++;
				if (GetEmptyInventorySlots(EmptyInventorySlotsArray) < SlotsRequired)
				{
					// Not enough slots, put the two hand weapon back in inventory and abort
					if (TwoHandRemovedFromInventory) AddItemToInventory(ItemToEquip->GetClass(), ReturnedItems, AddItemsMessage, ItemToEquip, 1);
					return EInventoryMessage::InventoryFull;
				}

				// Checks passed, unequip the current weapon slots
				EInventoryMessage UnEquip = this->UnEquip(ESlotType::OffHand);
				// Check if the item was successfully unequipped or the slot is empty
				if (UnEquip != EInventoryMessage::Success && UnEquip != EInventoryMessage::DoesNotExist)
				{
					return UnEquip; // Item could not be unequipped. Maybe not enough inventory space
				}
				// Swap the item to be equipped with the main hand
				if (EquipmentSlots.Contains(ESlotType::MainHand)) 
				{
					this->SwapEquipment(ItemToEquip, *EquipmentSlots.Find(ESlotType::MainHand));
					return EInventoryMessage::Success;
				}
			}

			// If item is to be equipped in offhand slot, unequip a two hand weapon if its there 
			if (SlotToEquip == ESlotType::OffHand)
			{
				if (EquipmentSlots.Contains(ESlotType::MainHand)) 
				{
					UEquipment* MainHandWeapon = *EquipmentSlots.Find(ESlotType::MainHand);
					if (MainHandWeapon->GetItemType() == EItemType::TwoHand) 
					{
						EInventoryMessage ReturnMessage = UnEquip(ESlotType::MainHand);
						if (ReturnMessage != EInventoryMessage::Success) 
						{
							return EInventoryMessage::InvalidEquipment;
						}
					}
				}

			}
			// Did not contain an equipped item already
			else 
			{
				if (ItemType == EItemType::ArmorModule || ItemType == EItemType::WeaponModule) 
				{
					return EInventoryMessage::InvalidEquipment; // Can't equip a module on an empty slot
				}
			}
			// Equip new item if not a module (module already equipped above)
			Cast<UItem>(ItemToEquip)->SetOwner(MyOwner); // Add the item owner
			if (ItemType != EItemType::ArmorModule && ItemType != EItemType::WeaponModule) 
			{
				if (!AddEquipmentToCharacter(ItemToEquip)) 
				{
					return EInventoryMessage::DoesNotExist;
				}
			}
			RemoveItemFromInventory(ItemToEquip);
			return EInventoryMessage::Success;
			
		}
		return EInventoryMessage::DoesNotExist;
	}
	return EInventoryMessage::DoesNotExist;
}

EInventoryMessage UEquipmentComponent::UnEquip(ESlotType SlotToUnequip)
{
	// Return storage containers for inventory operations
	TArray<UItem*> ReturnedItems;
	EInventoryMessage ReturnedMessage;
	
	// Verify there is at least one inventory slot available
	TArray<int32> EmptyInventorySlots;
	if (GetEmptyInventorySlots(EmptyInventorySlots) < 1) 
	{
		return EInventoryMessage::InventoryFull;
	}

	// Verify there is actually something to remove from this slot
	if (!EquipmentSlots.Contains(SlotToUnequip)) 
	{
		return EInventoryMessage::InvalidEquipment;
	}
	UEquipment* ItemToUnequip = *EquipmentSlots.Find(SlotToUnequip);
	
	// Attempt to remove the item
	if (!RemoveEquipmentFromCharacter(ItemToUnequip)) return EInventoryMessage::DoesNotExist;
	
	// Return the item to inventory	
	AddItemToInventory(ItemToUnequip->StaticClass(), ReturnedItems, ReturnedMessage, ItemToUnequip);
	return EInventoryMessage::Success;
}

EInventoryMessage UEquipmentComponent::SwapEquipment(UEquipment* Equipment1, UEquipment* Equipment2) 
{
	// Verify both items are valid
	if (Equipment1->IsValidLowLevel() && Equipment2->IsValidLowLevel())
	{
		// Verify we own both items
		if (Equipment1->GetOwner() == CastChecked<AMOBACharacter>(this->GetOwner()) && Equipment2->GetOwner() == CastChecked<AMOBACharacter>(this->GetOwner())) 
		{
			// Verify items use the same slot type
			if (Equipment1->GetEquipmentSlotType() == Equipment2->GetEquipmentSlotType())
			{
				// Verify that one of them is actually equipped already
				int32 SwapFirstItem = -1;
				if (EquipmentSlots.Contains(Equipment1->GetEquipmentSlotType())) 
				{
					SwapFirstItem = 1;
				}
				else if (EquipmentSlots.Contains(Equipment2->GetEquipmentSlotType()))
				{
					SwapFirstItem = 0;
				}
				else
				{
					return EInventoryMessage::WrongSlot;
				}
				UEquipment* ItemToRemove;
				UEquipment* ItemToAdd;
				switch (SwapFirstItem) 
				{
					case 0: 
					{
						ItemToAdd = Equipment2;
						ItemToRemove = Equipment1;
					}
					case 1:
					{
						ItemToAdd = Equipment1;
						ItemToRemove = Equipment2;
					}
					default: return EInventoryMessage::DoesNotExist;
				}

				// Remove first item and granted gameplay effects
				if (!RemoveEquipmentFromCharacter(ItemToRemove)) 
				{
					return EInventoryMessage::InvalidEquipment;
				}
				
				// Remove second item from inventory if it is in there
				if (ItemInstanceAlreadyPresentInInventory(CastChecked<UItem>(ItemToAdd)))
				{
					RemoveItemFromInventory(ItemToAdd);
				}
				// Equip second item and grant gameplay effects
				if (!AddEquipmentToCharacter(ItemToAdd)) 
				{
					// Operation failed, undo previous removal operation
					AddEquipmentToCharacter(ItemToRemove);
					return EInventoryMessage::InvalidEquipment;
				}
				// Add first item to inventory
				TArray<UItem*> ReturnedItems;
				EInventoryMessage Message;
				AddItemToInventory(CastChecked<UItem>(ItemToRemove)->GetClass(), ReturnedItems, Message, ItemToRemove, 1);
				return EInventoryMessage::Success;
			}
			else 
			{
				return EInventoryMessage::InvalidEquipment;
			}
		}
		// We don't own both items
		else 
		{
			return EInventoryMessage::InvalidEquipment;
		}
	}
	// One or more items is(are) not valid
	else 
	{
		return EInventoryMessage::DoesNotExist;
	}
}

bool UEquipmentComponent::AddEquipmentToCharacter(UEquipment* ItemToAdd) 
{
	// Verify item is a valid equippable item
	if (ItemToAdd->IsValidLowLevel()) 
	{
		ESlotType EquipmentSlotType = ItemToAdd->GetEquipmentSlotType();
		EItemType ItemType = ItemToAdd->GetItemType();
		AMOBACharacter* MyOwner = ItemToAdd->GetOwner();
		// Verify the slot is available
		if (!EquipmentSlots.Contains(EquipmentSlotType))
		{
			// Verify we own the item to be equipped
			if (ItemToAdd->GetOwner() == CastChecked<AMOBACharacter>(this->GetOwner())) 
			{
				// Add equipment to equipment slot, apply gameplay effects, and broadcast delegate
				EquipmentSlots.Add(EquipmentSlotType, ItemToAdd);
				for (auto Effect : Cast<UEquipment>(ItemToAdd)->GetGrantedEffects())
				{
					FGameplayEffectContextHandle Context = CastChecked<AMOBACharacter>(MyOwner)->AbilitySystemComponent->MakeEffectContext();
					CastChecked<AMOBACharacter>(MyOwner)->AbilitySystemComponent->BP_ApplyGameplayEffectToSelf(Effect.Key, Effect.Value, Context);
				}
				OnEquipmentChange.Broadcast((uint8)ItemToAdd->GetEquipmentSlotType(), ItemToAdd);
				if (ItemType == EItemType::OneHand || ItemType == EItemType::TwoHand) // Broadcast weapon attribute changes. Other attribute changes handled by attribute set
				{
					if (EquipmentSlotType == ESlotType::MainHand) MyOwner->AttributeSet->MainHandChange.Broadcast(MyOwner->AttributeSet->MainHandAttackSpeed.GetCurrentValue(), MyOwner->AttributeSet->MainHandMinDamage.GetCurrentValue(), MyOwner->AttributeSet->MainHandMaxDamage.GetCurrentValue(), MyOwner->AttributeSet->MainHandAttackRange.GetCurrentValue());
					else if (EquipmentSlotType == ESlotType::OffHand) MyOwner->AttributeSet->OffHandChange.Broadcast(MyOwner->AttributeSet->OffHandAttackSpeed.GetCurrentValue(), MyOwner->AttributeSet->OffHandMinDamage.GetCurrentValue(), MyOwner->AttributeSet->OffHandMaxDamage.GetCurrentValue(), MyOwner->AttributeSet->OffHandAttackRange.GetCurrentValue());
				}
				return true;
			}
		}
	}
	return false;
}

bool UEquipmentComponent::RemoveEquipmentFromCharacter(UEquipment* ItemToRemove) 
{
	// Verify equipment pointer is valid
	if (ItemToRemove->IsValidLowLevel()) 
	{
		ESlotType EquipmentSlotType = ItemToRemove->GetEquipmentSlotType();
		EItemType ItemType = ItemToRemove->GetItemType();
		AMOBACharacter* MyOwner = ItemToRemove->GetOwner();
		// Verify that the item is actually equipped already
		if (EquipmentSlots.Contains(EquipmentSlotType))
		{
			if (*EquipmentSlots.Find(EquipmentSlotType) == ItemToRemove)
			{
				EquipmentSlots.Remove(EquipmentSlotType);
				for (auto Effect : Cast<UEquipment>(ItemToRemove)->GetGrantedEffects())
				{
					FGameplayEffectContextHandle Context = MyOwner->AbilitySystemComponent->MakeEffectContext();
					MyOwner->AbilitySystemComponent->RemoveActiveGameplayEffectBySourceEffect(Effect.Key, ItemToRemove->GetOwner()->AbilitySystemComponent);
				}
				OnEquipmentChange.Broadcast((uint8)EquipmentSlotType, ItemToRemove);
				if (ItemType == EItemType::OneHand || ItemType == EItemType::TwoHand) // Broadcast weapon attribute changes. Other attribute changes handled by attribute set
				{
					if (EquipmentSlotType == ESlotType::MainHand) MyOwner->AttributeSet->MainHandChange.Broadcast(MyOwner->AttributeSet->MainHandAttackSpeed.GetCurrentValue(), MyOwner->AttributeSet->MainHandMinDamage.GetCurrentValue(), MyOwner->AttributeSet->MainHandMaxDamage.GetCurrentValue(), MyOwner->AttributeSet->MainHandAttackRange.GetCurrentValue());
					else if (EquipmentSlotType == ESlotType::OffHand) MyOwner->AttributeSet->OffHandChange.Broadcast(MyOwner->AttributeSet->OffHandAttackSpeed.GetCurrentValue(), MyOwner->AttributeSet->OffHandMinDamage.GetCurrentValue(), MyOwner->AttributeSet->OffHandMaxDamage.GetCurrentValue(), MyOwner->AttributeSet->OffHandAttackRange.GetCurrentValue());
				}
				return true;
			}
		}
	}
	return false;
}