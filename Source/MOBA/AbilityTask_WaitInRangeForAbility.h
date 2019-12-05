// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilityTask.h"
#include "UObject/ObjectMacros.h"
#include "Engine/EngineTypes.h"
#include "MOBACharacter.h"
#include "Components/SphereComponent.h"
#include "AIModule/Classes/AIController.h"
#include "AbilityTask_WaitInRangeForAbility.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWaitInRangeForAbility);

UCLASS()
class MOBA_API UAbilityTask_WaitInRangeForAbility : public UAbilityTask
{
	GENERATED_BODY()
	
	UAbilityTask_WaitInRangeForAbility(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintAssignable)
		FWaitInRangeForAbility OnInRange;

	FString GetDebugString() const override;

	AMOBACharacter* SourceCharacter;
	AMOBACharacter* TargetCharacter;
	USphereComponent* RangeSphere;
	AAIController* AIController;
	FVector TargetLocation = FVector{ 0.0f,0.0f,0.0f };

	UFUNCTION()
	void OverlapHandle(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
	
	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (DisplayName = "WaitForInRange", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
		static UAbilityTask_WaitInRangeForAbility* WaitInRangeForTargetedAbility(UGameplayAbility* OwningAbility, AMOBACharacter* Source, AMOBACharacter* Target, USphereComponent* InSphere, FVector InLocation);

	virtual void InitSimulatedTask(UGameplayTasksComponent& InGameplayTasksComponent) override;
	
	virtual void Activate() override;

	virtual void OnDestroy(bool AbilityIsEnding) override;
};
