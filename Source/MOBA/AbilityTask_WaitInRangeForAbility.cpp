// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityTask_WaitInRangeForAbility.h"

UAbilityTask_WaitInRangeForAbility::UAbilityTask_WaitInRangeForAbility(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void UAbilityTask_WaitInRangeForAbility::OverlapHandle(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) 
{
	AMOBACharacter* Target = Cast<AMOBACharacter>(OtherActor);
	// Check if the OtherActor is our Target
	if (TargetCharacter == Target)
	{	
		if (OtherComp == Cast<UPrimitiveComponent>(Target->GetMesh())) 
		{
			AIController->StopMovement();
			// We found the target, broadcast the delegate and end the task
			if (ShouldBroadcastAbilityTaskDelegates()) {
				OnInRange.Broadcast();
			}
			EndTask();
		}
		
	}
}

UAbilityTask_WaitInRangeForAbility* UAbilityTask_WaitInRangeForAbility::WaitInRangeForTargetedAbility(UGameplayAbility * OwningAbility, AMOBACharacter* Source, AMOBACharacter* Target, USphereComponent* InSphere, FVector InLocation)
{

	UAbilityTask_WaitInRangeForAbility* MyObj = NewAbilityTask<UAbilityTask_WaitInRangeForAbility>(OwningAbility);
	MyObj->SourceCharacter = Source;
	MyObj->TargetCharacter = Target;
	MyObj->RangeSphere = InSphere;
	MyObj->AIController = Cast<AAIController>(Source->GetController());
	if (InLocation != FVector{})	MyObj->TargetLocation = InLocation;
	return MyObj;
}

void UAbilityTask_WaitInRangeForAbility::InitSimulatedTask(UGameplayTasksComponent& InGameplayTasksComponent)
{
	Super::InitSimulatedTask(InGameplayTasksComponent);
}

void UAbilityTask_WaitInRangeForAbility::Activate()
{
	AAIController* AIController;
	if (RangeSphere)
	{
		RangeSphere->OnComponentBeginOverlap.AddDynamic(this, &UAbilityTask_WaitInRangeForAbility::OverlapHandle);
	}
	AIController = Cast<AAIController>(SourceCharacter->GetController());
	if (AIController) 
	{
		if (TargetCharacter) 
		{
			AIController->MoveToActor(TargetCharacter, 2.0f, false, true, false);
		}
		else 
		{
			if (TargetLocation != FVector{ 0.0f,0.0f,0.0f }) AIController->MoveToLocation(TargetLocation, 2.0f, true, true, false, false);
		}
		
	}
	SetWaitingOnAvatar();
}

void UAbilityTask_WaitInRangeForAbility::OnDestroy(bool AbilityIsEnding)
{
	Super::OnDestroy(AbilityIsEnding);
	if (RangeSphere)
	{
		RangeSphere->OnComponentBeginOverlap.RemoveDynamic(this, &UAbilityTask_WaitInRangeForAbility::OverlapHandle);
	}
	ClearWaitingOnAvatar();
}

FString UAbilityTask_WaitInRangeForAbility::GetDebugString() const
{
	return FString::Printf(TEXT("InRangeForAbility"));
}