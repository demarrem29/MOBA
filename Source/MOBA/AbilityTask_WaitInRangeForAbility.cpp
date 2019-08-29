// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityTask_WaitInRangeForAbility.h"

UAbilityTask_WaitInRangeForAbility::UAbilityTask_WaitInRangeForAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void UAbilityTask_WaitInRangeForAbility::OverlapHandle(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) 
{
	if (OtherActor)
	{
		// Find Source Actor Enemy Target
		AMOBACharacter* SourceCharacter = Cast<AMOBACharacter>(OverlappedComponent->GetAttachmentRootActor());
		if (SourceCharacter) 
		{
			// Find the Target
			AMOBACharacter* TargetCharacter = SourceCharacter->MyEnemyTarget;
			if (TargetCharacter) 
			{
				// Check if target matches the overlap event
				if (OtherActor == TargetCharacter) 
				{
					// We found the target, broadcast the delegate and end the task
					if (ShouldBroadcastAbilityTaskDelegates()) {
						OnInRange.Broadcast();
					}
					EndTask();
				}
			}
		}
	}
}

UAbilityTask_WaitInRangeForAbility* UAbilityTask_WaitInRangeForAbility::WaitInRangeForAbility(UGameplayAbility * OwningAbility, AMOBACharacter* Source, AMOBACharacter* Target)
{

	UAbilityTask_WaitInRangeForAbility* MyObj = crea<UAbilityTask_WaitInRangeForAbility>(OwningAbility);
	return MyObj;
}

void UAbilityTask_WaitInRangeForAbility::InitSimulatedTask(UGameplayTasksComponent& InGameplayTasksComponent)
{
	Super::InitSimulatedTask(InGameplayTasksComponent);
}

void UAbilityTask_WaitInRangeForAbility::Activate()
{
	USphereComponent* RangeSphere = GetComponent();
	if (RangeSphere)
	{
		RangeSphere->OnComponentBeginOverlap.AddDynamic(this, &UAbilityTask_WaitInRangeForAbility::OverlapHandle);
	}
	SetWaitingOnAvatar();
}

void UAbilityTask_WaitInRangeForAbility::OnDestroy(bool AbilityIsEnding)
{
	Super::OnDestroy(AbilityIsEnding);
	USphereComponent* RangeSphere = GetComponent();
	if (RangeSphere)
	{
		RangeSphere->OnComponentBeginOverlap.RemoveDynamic(this, &UAbilityTask_WaitInRangeForAbility::OverlapHandle);
	}
}

FString UAbilityTask_WaitInRangeForAbility::GetDebugString() const
{
	return FString::Printf(TEXT("InRangeForAbility"));
}

USphereComponent* UAbilityTask_WaitInRangeForAbility::GetComponent() 
{
	USphereComponent* SphereComponent = nullptr;
	AActor* ActorOwner = GetAvatarActor();
	if (ActorOwner)
	{
		SphereComponent = Cast<USphereComponent>(ActorOwner->FindComponentByClass<USphereComponent>());
		if (!SphereComponent)
		{
			return nullptr;
		}
		else return SphereComponent;
	}
	else return nullptr;
}