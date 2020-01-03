// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "MOBACharacter.h"
#include "MOBAAttributeSet.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "MOBAGameplayAbility.h"
#include "GameplayTagContainer.h"

AMOBACharacter::AMOBACharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 1280.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>("Ability System Component");
	AttributeSet = CreateDefaultSubobject<UMOBAAttributeSet>("Attribute Set");
	EquipmentComponent = CreateDefaultSubobject<UEquipmentComponent>("Equipment");

	// Set Default Combat Values
	bIsAttacking = false;
	bIsInCombat = false;

	// Configure Projectile Target: Provides a height offset for homing projectiles to target.
	ProjectileTarget = CreateDefaultSubobject<USceneComponent>("Projectile Target Component");
	ProjectileTarget->SetupAttachment(RootComponent);
	ProjectileTarget->SetRelativeLocation(FVector{ 0,0,50 });

	// Initialize Any Attributes that require additional logic outside of default attribute values
	ACharacter* Char = Cast<ACharacter>(GetParentActor());
	if (AttributeSet)
	{
		if (Char) {
			Char->GetCharacterMovement()->MaxWalkSpeed = AttributeSet->MovementSpeed.GetCurrentValue(); // Set walk speed to value in the attribute set
		}
	}
}

void AMOBACharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
}

UAbilitySystemComponent* AMOBACharacter::GetAbilitySystemComponent() const 
{
	return AbilitySystemComponent;
}

void AMOBACharacter::AcquireAbility(TSubclassOf<UGameplayAbility> AbilityToAcquire) 
{
	if (AbilitySystemComponent) 
	{
		if (HasAuthority() && AbilityToAcquire)
		{
			FGameplayAbilitySpecDef SpecDef = FGameplayAbilitySpecDef();
			SpecDef.Ability = AbilityToAcquire;
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(SpecDef, 1);
			AbilitySystemComponent->GiveAbility(AbilitySpec);
			UMOBAGameplayAbility*  MOBAGameplayAbility = Cast<UMOBAGameplayAbility>(AbilityToAcquire);
		}
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		
	}
}

void AMOBACharacter::RemoveAbility(TSubclassOf<UGameplayAbility> AbilityToRemove) 
{
	if (AbilitySystemComponent)
	{
		if (HasAuthority() && AbilityToRemove)
		{
			FGameplayAbilitySpec* AbilitySpec = AbilitySystemComponent->FindAbilitySpecFromClass(AbilityToRemove);
			if (AbilitySpec) 
			{
				FGameplayAbilitySpecHandle SpecHandle = AbilitySpec->Handle;
				AbilitySystemComponent->ClearAbility(SpecHandle);
			}
		}
		AbilitySystemComponent->RefreshAbilityActorInfo();
	}
}

// Check if we can perform a basic attack
float AMOBACharacter::GetBasicAttackCooldown() 
{
	if (AbilitySystemComponent) 
	{
		FGameplayTagContainer CooldownContainer;
		FGameplayTag CooldownTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Abilities.Basic.BasicAttack.Cooldown")));
		CooldownContainer.AddTag(CooldownTag);
		FGameplayEffectQuery MyQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownContainer);
		if (AbilitySystemComponent->GetActiveEffectsTimeRemaining(MyQuery).Num() > 0) 
		{
			return (AbilitySystemComponent->GetActiveEffectsTimeRemaining(MyQuery)).Last();
		}
	}
	return 0.0f;
}

// Check if the item (if any) in the offhand slot is a weapon (is off hand basic attack allowed?)
bool AMOBACharacter::GetOffHandWeaponEquipped() 
{
	if (!EquipmentComponent) return false;
	if (EquipmentComponent->EquipmentSlots.Contains(ESlotType::OffHand)) 
	{
		UEquipment* OffHandEquipment = *EquipmentComponent->EquipmentSlots.Find(ESlotType::OffHand);
		UWeapon* OffHandWeapon = Cast<UWeapon>(OffHandEquipment);
		if (OffHandWeapon) 
		{
			return true;
		}
	}
	return false;
}

// Check and see if another character is hostile (should we allow attacks or abilities on this target)
bool AMOBACharacter::IsHostile(AMOBACharacter* TargetCharacter)
{
	if (TargetCharacter->IsValidLowLevel() && this->IsValidLowLevel())
	{
		if (this->MyTeam != TargetCharacter->MyTeam && TargetCharacter->MyTeam != ETeam::NeutralFriendly)
		{
			return true;
		}
		else return false;
	}
	else return false;
}

void AMOBACharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) 
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	AbilitySystemComponent->BindAbilityActivationToInputComponent(PlayerInputComponent, FGameplayAbilityInputBinds("ConfirmInput", "CancelInput", "AbilityInput"));
}

void AMOBACharacter::BeginPlay() 
{
	Super::BeginPlay();
	if (AttributeSet) {
		CombatStatusChangeDelegate.AddDynamic(this, &AMOBACharacter::CombatStatusChange);
		AttributeSet->PhysicalDamageReduction = AttributeSet->CalculateDamageReduction(AttributeSet->Armor.GetCurrentValue());
		AttributeSet->EnvironmentalDamageReduction = AttributeSet->CalculateDamageReduction(AttributeSet->EnvironmentalResistance.GetCurrentValue());
		AttributeSet->HealthChange.AddDynamic(this, &AMOBACharacter::HealthChange);
		AttributeSet->HealthRegenChange.AddDynamic(this, &AMOBACharacter::HealthRegenChange);
		AttributeSet->HealingModifierChange.AddDynamic(this, &AMOBACharacter::HealingModifierChange);
		AttributeSet->ManaChange.AddDynamic(this, &AMOBACharacter::ManaChange);
		AttributeSet->ManaRegenChange.AddDynamic(this, &AMOBACharacter::ManaRegenChange);
		AttributeSet->LevelChange.AddDynamic(this, &AMOBACharacter::LevelChange);
		AttributeSet->ExperienceChange.AddDynamic(this, &AMOBACharacter::ExperienceChange);
		AttributeSet->AttackPowerChange.AddDynamic(this, &AMOBACharacter::AttackPowerChange);
		AttributeSet->SpellPowerChange.AddDynamic(this, &AMOBACharacter::SpellPowerChange);
		AttributeSet->MainHandChange.AddDynamic(this, &AMOBACharacter::MainHandChange);
		AttributeSet->OffHandChange.AddDynamic(this, &AMOBACharacter::OffHandChange);		
		AttributeSet->BonusAttackSpeedChange.AddDynamic(this, &AMOBACharacter::BonusAttackSpeedChange);
		AttributeSet->CriticalChanceChange.AddDynamic(this, &AMOBACharacter::CriticalChanceChange);
		AttributeSet->CriticalDamageChange.AddDynamic(this, &AMOBACharacter::CriticalDamageChange);
		AttributeSet->ArmorChange.AddDynamic(this, &AMOBACharacter::ArmorChange);
		AttributeSet->PhysicalDamageReductionChange.AddDynamic(this, &AMOBACharacter::PhysicalDamageReductionChange);
		AttributeSet->EnvironmentalResistanceChange.AddDynamic(this, &AMOBACharacter::EnvironmentalResistanceChange);
		AttributeSet->EnvironmentalDamageReductionChange.AddDynamic(this, &AMOBACharacter::EnvironmentalDamageReductionChange);
		AttributeSet->FlatDamageReductionChange.AddDynamic(this, &AMOBACharacter::FlatDamageReductionChange);
		AttributeSet->MovementSpeedChange.AddDynamic(this, &AMOBACharacter::MovementSpeedChange);
	}
	if (AbilitySystemComponent) 
	{
		AbilitySystemComponent->OnAbilityEnded.AddUObject(this, &AMOBACharacter::OnAbilityEnded);
		FOnGivenActiveGameplayEffectRemoved* GameplayEffectRemovedDelegate = &AbilitySystemComponent->OnAnyGameplayEffectRemovedDelegate();
		GameplayEffectRemovedDelegate->AddUObject(this, &AMOBACharacter::OnGameplayEffectEnd);
	}
	if (EquipmentComponent) 
	{
		EquipmentComponent->OnInventoryChange.AddDynamic(this, &AMOBACharacter::InventoryChange);
		EquipmentComponent->OnEquipmentChange.AddDynamic(this, &AMOBACharacter::EquipmentChange);
	}
}

void AMOBACharacter::PossessedBy(AController* NewController) 
{
	Super::PossessedBy(NewController);
	AbilitySystemComponent->RefreshAbilityActorInfo();
}

void AMOBACharacter::InventoryChange(TArray<UItem*> AffectedSlots, TArray<int32> AffectedIndices)
{
	BP_InventoryChange(AffectedSlots, AffectedIndices);
}
void AMOBACharacter::EquipmentChange(uint8 AffectedSlot, UEquipment* EquipmentObjRef)
{
	BP_EquipmentChange(ESlotType(AffectedSlot), EquipmentObjRef);
}
void AMOBACharacter::HealthChange(FGameplayAttributeData health, FGameplayAttributeData maxhealth) 
{
	BP_HealthChange(health, maxhealth);
}
void AMOBACharacter::HealthRegenChange(FGameplayAttributeData HealthRegen)
{
	BP_HealthRegenChange(HealthRegen);
}
void AMOBACharacter::HealingModifierChange(FGameplayAttributeData HealingModifier)
{
	BP_HealingModifierChange(HealingModifier);
}
void AMOBACharacter::ManaChange(FGameplayAttributeData mana, FGameplayAttributeData maxmana)
{
	BP_ManaChange(mana, maxmana);
}
void AMOBACharacter::ManaRegenChange(FGameplayAttributeData ManaRegen)
{
	BP_ManaRegenChange(ManaRegen);
}
void AMOBACharacter::LevelChange(FGameplayAttributeData level, FGameplayAttributeData maxlevel)
{
	BP_LevelChange(level, maxlevel);
}
void AMOBACharacter::ExperienceChange(FGameplayAttributeData experience, FGameplayAttributeData maxexperience)
{
	BP_ExperienceChange(experience, maxexperience);
}
void AMOBACharacter::AttackPowerChange(FGameplayAttributeData AttackPower)
{
	BP_AttackPowerChange(AttackPower);
}
void AMOBACharacter::SpellPowerChange(FGameplayAttributeData SpellPower)
{
	BP_SpellPowerChange(SpellPower);
}
void AMOBACharacter::MainHandChange(FGameplayAttributeData MainHandAttackSpeed, FGameplayAttributeData MainHandMinDamage, FGameplayAttributeData MainHandMaxDamage, FGameplayAttributeData MainHandAttackRange)
{
	BP_MainHandChange(MainHandAttackSpeed, MainHandMinDamage,MainHandMaxDamage, MainHandAttackRange);
}
void AMOBACharacter::OffHandChange(FGameplayAttributeData OffHandAttackSpeed, FGameplayAttributeData OffHandMinDamage, FGameplayAttributeData OffHandMaxDamage, FGameplayAttributeData OffHandAttackRange)
{
	BP_OffHandChange(OffHandAttackSpeed, OffHandMinDamage, OffHandMaxDamage, OffHandAttackRange);
}
void AMOBACharacter::BonusAttackSpeedChange(FGameplayAttributeData BonusAttackSpeed)
{
	BP_BonusAttackSpeedChange(BonusAttackSpeed);
}
void AMOBACharacter::CriticalChanceChange(FGameplayAttributeData CriticalChance)
{
	BP_CriticalChanceChange(CriticalChance);
}
void AMOBACharacter::CriticalDamageChange(FGameplayAttributeData CriticalDamage)
{
	BP_CriticalDamageChange(CriticalDamage);
}

void AMOBACharacter::ArmorChange(FGameplayAttributeData Armor)
{
	AttributeSet->PhysicalDamageReduction.SetCurrentValue(AttributeSet->CalculateDamageReduction(Armor.GetCurrentValue()));
	BP_ArmorChange(Armor);
}

void AMOBACharacter::PhysicalDamageReductionChange(FGameplayAttributeData PhysicalDamageReduction)
{
	BP_PhysicalDamageReductionChange(PhysicalDamageReduction);
}

void AMOBACharacter::EnvironmentalResistanceChange(FGameplayAttributeData EnvironmentalResistance)
{
	AttributeSet->EnvironmentalDamageReduction.SetCurrentValue(AttributeSet->CalculateDamageReduction(EnvironmentalResistance.GetCurrentValue()));
	BP_EnvironmentalResistanceChange(EnvironmentalResistance);
}

void AMOBACharacter::EnvironmentalDamageReductionChange(FGameplayAttributeData EnvironmentalDamageReduction)
{
	BP_EnvironmentalDamageReductionChange(EnvironmentalDamageReduction);
}

void AMOBACharacter::FlatDamageReductionChange(FGameplayAttributeData FlatDamageReduction)
{
	BP_FlatDamageReductionChange(FlatDamageReduction);
}

void AMOBACharacter::MovementSpeedChange(FGameplayAttributeData MovementSpeed)
{
	ACharacter* Char = Cast<ACharacter>(GetParentActor());
	if (Char) {
		Char->GetCharacterMovement()->MaxWalkSpeed = MovementSpeed.GetCurrentValue();
	}
	BP_MovementSpeedChange(MovementSpeed);
}

void AMOBACharacter::CombatStatusChange(bool bIsAttackingIn, bool bIsInCombatIn) 
{
	if (bIsInCombatIn) 
	{
		UWorld* World = GetWorld();
		if (World) 
		{
			World->GetTimerManager().SetTimer(CombatTimerHandle, this, &AMOBACharacter::CombatTimerCallback, 5.0f, false);
		}
	}
	else 
	{
		bUseOffHandWeapon = false;
	}
	BP_CombatStatusChange(bIsAttackingIn, bIsInCombatIn);
}

void AMOBACharacter::OnAbilityEnded(const FAbilityEndedData& AbilityEndData)
{
	BP_OnAbilityEnded(AbilityEndData);
}

void AMOBACharacter::OnGameplayEffectEnd(const FActiveGameplayEffect& EndedGameplayEffect)
{
	// Check if the effect was a basic attack cooldown
	FGameplayTagContainer BasicAttackCooldownTagContainer;
	FGameplayTagContainer EndedGameplayEffectContainer;
	FGameplayTag BasicAttackCooldown;
	BasicAttackCooldown = FGameplayTag::RequestGameplayTag(FName("Abilities.Basic.BasicAttack.Cooldown"));
	BasicAttackCooldownTagContainer.AddTag(BasicAttackCooldown);
	EndedGameplayEffect.Spec.GetAllGrantedTags(EndedGameplayEffectContainer);
	if (EndedGameplayEffectContainer.HasAny(BasicAttackCooldownTagContainer)) 
	{
		// Basic Attack Cooldown Complete, try basic attack
		TryBasicAttack();
	}	
	BP_OnGameplayEffectEnd(EndedGameplayEffect);
}

void AMOBACharacter::TryBasicAttack()
{
	if (bIsAttacking && MyEnemyTarget) 
	{
		float cooldownremaining = GetBasicAttackCooldown();
		if (cooldownremaining <= 0)
		{
			if (GetOffHandWeaponEquipped())
			{
				BP_TryBasicAttack(bUseOffHandWeapon);
				bUseOffHandWeapon = !bUseOffHandWeapon;
			}
			else BP_TryBasicAttack(false);
		}
		else
		{
			AAIController* MyAIController = Cast<AAIController>(GetController());
			if (MyAIController) 
			{
				MyAIController->MoveToActor(MyEnemyTarget, 5.0f, false, true, false);
			}
		}
	}
}

void AMOBACharacter::CombatTimerCallback() 
{
	bIsInCombat = false;
	CombatStatusChangeDelegate.Broadcast(bIsAttacking, bIsInCombat);
}