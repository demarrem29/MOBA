// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilityTask_WaitForProjectileHit.h"

UAbilityTask_WaitForProjectileHit::UAbilityTask_WaitForProjectileHit(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UAbilityTask_WaitForProjectileHit::OnDestroyed(AActor* Actor, EEndPlayReason::Type EndPlayReason)
{
	if (Actor == Cast<AActor>(MyProjectile)) 
	{
		// Projectile was destroyed
		if (ShouldBroadcastAbilityTaskDelegates()) {
			OnProjectileHit.Broadcast();
		}
		EndTask();
	}
}

UAbilityTask_WaitForProjectileHit* UAbilityTask_WaitForProjectileHit::WaitForProjectileHit(UGameplayAbility* OwningAbility, AProjectile* SourceProjectile)
{

	UAbilityTask_WaitForProjectileHit* MyObj = NewAbilityTask<UAbilityTask_WaitForProjectileHit>(OwningAbility);
	if (MyObj)
	{
		MyObj->MyProjectile = SourceProjectile;
	}
	return MyObj;
}

void UAbilityTask_WaitForProjectileHit::Activate()
{
	if (MyProjectile) 
	{
		MyProjectile->OnEndPlay.AddDynamic(this, &UAbilityTask_WaitForProjectileHit::OnDestroyed);
	}
}

void UAbilityTask_WaitForProjectileHit::OnDestroy(bool AbilityIsEnding)
{
	Super::OnDestroy(AbilityIsEnding);
	if (MyProjectile)
	{
		MyProjectile->OnEndPlay.RemoveDynamic(this, &UAbilityTask_WaitForProjectileHit::OnDestroyed);
	}
}