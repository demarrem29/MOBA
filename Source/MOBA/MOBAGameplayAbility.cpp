// Fill out your copyright notice in the Description page of Project Settings.


#include "MOBAGameplayAbility.h"

// Function to check distance and whether the ability can be cast before moving
bool UMOBAGameplayAbility::InRangeForAbility(FVector TargetLocation, AMOBACharacter* TargetCharacter) 
{
	if (RangeSphere) 
	{
		if (TargetCharacter) return RangeSphere->IsOverlappingComponent(TargetCharacter->GetMesh());
		else 
		{
			FVector MyLocation = RangeSphere->GetComponentLocation();
			return (FVector::Dist(MyLocation, TargetLocation) <= AbilityRange);
		}
	}
	return false;
}

void UMOBAGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) 
{
	Super::OnGiveAbility(ActorInfo, Spec);
	AActor* MyOwner = GetOwningActorFromActorInfo();
	MyCharacter = Cast<AMOBACharacter>(MyOwner);
	if (bPassiveAbility) 
	{
		if (MyCharacter)
		{
			MyCharacter->AbilitySystemComponent->TryActivateAbility(GetCurrentAbilitySpecHandle(), true);
		}
	}
}

void UMOBAGameplayAbility::PreActivate(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, FOnGameplayAbilityEnded::FDelegate* OnGameplayAbilityEndedDelegate) 
{
	Super::PreActivate(Handle, ActorInfo, ActivationInfo, OnGameplayAbilityEndedDelegate);
	if (MyCharacter)
	{
		RangeSphere = NewObject<USphereComponent>(CastChecked<UObject>(MyCharacter),FName("Ability Range Sphere"));
		RangeSphere->AttachToComponent(MyCharacter->GetRootComponent(), FAttachmentTransformRules{ EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, EAttachmentRule::SnapToTarget, false });
		RangeSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
		RangeSphere->bHiddenInGame = true;
		RangeSphere->SetSphereRadius(AbilityRange);
		RangeSphere->bMultiBodyOverlap = 1;
		RangeSphere->SetGenerateOverlapEvents(true);
		MyCharacter->FinishAndRegisterComponent(RangeSphere);
	}
	OnGameplayAbilityEnded.AddUObject(this, &UMOBAGameplayAbility::OnAbilityEnded);
}

void UMOBAGameplayAbility::OnAbilityEnded(UGameplayAbility* InAbility) 
{
	MyCharacter->RemoveOwnedComponent(RangeSphere);
}