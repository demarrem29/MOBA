// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "Projectile.h"
#include "AbilityTask_WaitForProjectileHit.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FWaitForProjectileHit);

UCLASS()
class MOBA_API UAbilityTask_WaitForProjectileHit : public UAbilityTask
{
	GENERATED_BODY()

	UAbilityTask_WaitForProjectileHit(const FObjectInitializer& ObjectInitializer);

	UPROPERTY(BlueprintAssignable)
		FWaitForProjectileHit OnProjectileHit;

	UPROPERTY(VisibleAnywhere)
		AProjectile* MyProjectile;

	UFUNCTION()
		void OnDestroyed(AActor* Actor, EEndPlayReason::Type EndPlayReason);

	UFUNCTION(BlueprintCallable, Category = "Ability|Tasks", meta = (DisplayName = "WaitForProjectileHit", HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "TRUE"))
		static UAbilityTask_WaitForProjectileHit* WaitForProjectileHit(UGameplayAbility* OwningAbility, AProjectile* SourceProjectile);
	
	virtual void Activate() override;

	virtual void OnDestroy(bool AbilityIsEnding) override;
};
