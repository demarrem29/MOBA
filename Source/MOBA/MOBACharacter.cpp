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

	// Set Default Combat Values
	bIsAttacking = false;
	bIsInCombat = false;
	RangeDetector = CreateDefaultSubobject<USphereComponent>("Attack Range Indicator");
	RangeDetector->SetGenerateOverlapEvents(true);
	RangeDetector->SetupAttachment(RootComponent);
	RangeDetector->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
	RangeDetector->bHiddenInGame = true;
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
		}
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
	
}

float AMOBACharacter::GetBasicAttackCooldown() 
{
	if (AbilitySystemComponent) 
	{
		FGameplayTagContainer CooldownContainer;
		FGameplayTag CooldownTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Abilities.BasicAttack.Cooldown")));
		CooldownContainer.AddTag(CooldownTag);
		FGameplayEffectQuery MyQuery = FGameplayEffectQuery::MakeQuery_MatchAnyOwningTags(CooldownContainer);
		if (AbilitySystemComponent->GetActiveEffectsTimeRemaining(MyQuery).Num() > 0) 
		{
			return (AbilitySystemComponent->GetActiveEffectsTimeRemaining(MyQuery)).Last();
		}
	}
	return 0.0f;
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
		RangeDetector->SetSphereRadius(AttributeSet->AttackRange.GetCurrentValue());
		AttributeSet->PhysicalDamageReduction = AttributeSet->CalculateDamageReduction(AttributeSet->Armor.GetCurrentValue());
		AttributeSet->EnvironmentalDamageReduction = AttributeSet->CalculateDamageReduction(AttributeSet->EnvironmentalResistance.GetCurrentValue());
		AttributeSet->HealthChange.AddDynamic(this, &AMOBACharacter::HealthChange);
		AttributeSet->HealthRegenChange.AddDynamic(this, &AMOBACharacter::HealthRegenChange);
		AttributeSet->ManaChange.AddDynamic(this, &AMOBACharacter::ManaChange);
		AttributeSet->ManaRegenChange.AddDynamic(this, &AMOBACharacter::ManaRegenChange);
		AttributeSet->LevelChange.AddDynamic(this, &AMOBACharacter::LevelChange);
		AttributeSet->ExperienceChange.AddDynamic(this, &AMOBACharacter::ExperienceChange);
		AttributeSet->AttackPowerChange.AddDynamic(this, &AMOBACharacter::AttackPowerChange);
		AttributeSet->SpellPowerChange.AddDynamic(this, &AMOBACharacter::SpellPowerChange);
		AttributeSet->AttackSpeedChange.AddDynamic(this, &AMOBACharacter::AttackSpeedChange);
		AttributeSet->CriticalChanceChange.AddDynamic(this, &AMOBACharacter::CriticalChanceChange);
		AttributeSet->CriticalDamageChange.AddDynamic(this, &AMOBACharacter::CriticalDamageChange);
		AttributeSet->AttackRangeChange.AddDynamic(this, &AMOBACharacter::AttackRangeChange);
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
}

void AMOBACharacter::PossessedBy(AController* NewController) 
{
	Super::PossessedBy(NewController);
	AbilitySystemComponent->RefreshAbilityActorInfo();

}

void AMOBACharacter::HealthChange(FGameplayAttributeData health, FGameplayAttributeData maxhealth) 
{
	BP_HealthChange(health, maxhealth);
}
void AMOBACharacter::HealthRegenChange(FGameplayAttributeData HealthRegen)
{
	BP_HealthRegenChange(HealthRegen);
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
void AMOBACharacter::AttackSpeedChange(FGameplayAttributeData AttackSpeed)
{
	BP_AttackSpeedChange(AttackSpeed);
}
void AMOBACharacter::CriticalChanceChange(FGameplayAttributeData CriticalChance)
{
	BP_CriticalChanceChange(CriticalChance);
}
void AMOBACharacter::CriticalDamageChange(FGameplayAttributeData CriticalDamage)
{
	BP_CriticalDamageChange(CriticalDamage);
}

void AMOBACharacter::AttackRangeChange(FGameplayAttributeData AttackRange)
{
	BP_AttackRangeChange(AttackRange);
}

void AMOBACharacter::ArmorChange(FGameplayAttributeData Armor)
{
	BP_ArmorChange(Armor);
}

void AMOBACharacter::PhysicalDamageReductionChange(FGameplayAttributeData PhysicalDamageReduction)
{
	BP_PhysicalDamageReductionChange(PhysicalDamageReduction);
}

void AMOBACharacter::EnvironmentalResistanceChange(FGameplayAttributeData EnvironmentalResistance)
{
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
	BP_CombatStatusChange(bIsAttackingIn, bIsInCombatIn);
}

void AMOBACharacter::OnAbilityEnded(const FAbilityEndedData& AbilityEndData)
{
	BP_OnAbilityEnded(AbilityEndData);
}

void AMOBACharacter::OnGameplayEffectEnd(const FActiveGameplayEffect& EndedGameplayEffect)
{
	
	FName gameplayeffectname = EndedGameplayEffect.Spec.Def->GetFName();
	// Check if the ended effect was the basic attack cooldown
	if (gameplayeffectname == TEXT("Default__GE_BasicAttackCooldown_C"))
	{
		// Basic Attack Cooldown Complete, try basic attack
		TryBasicAttack();
	}
	
	BP_OnGameplayEffectEnd(EndedGameplayEffect);
}

void AMOBACharacter::TryBasicAttack()
{
	float cooldownremaining = GetBasicAttackCooldown();
	if (cooldownremaining <= 0)
	{
		BP_TryBasicAttack();
	}
}