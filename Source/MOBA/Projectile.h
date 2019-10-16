// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "Components/SphereComponent.h"
#include "MOBACharacter.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Projectile.generated.h"

/**
 * 
 */

UCLASS(Blueprintable)
class MOBA_API AProjectile : public AActor
{

	GENERATED_BODY()
public:
	// Sets default values for this actor's properties
	AProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
		AMOBACharacter* MyEnemyTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Targeting")
		bool bIsInitialized = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
		bool bIsSingleTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Targeting")
		FVector SpawnedLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Targeting")
		FVector TargetLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Targeting")
		float MaxDistance;

	// Sphere collision component.
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
		USphereComponent* CollisionComponent;

	// Projectile movement component.
	UPROPERTY(VisibleAnywhere, Category = Movement)
		UProjectileMovementComponent* ProjectileMovementComponent;

	UFUNCTION(BlueprintImplementableEvent)
		void OnTargetReached(AMOBACharacter* InTarget);
	
	UFUNCTION(BlueprintCallable)
		void InitializeProjectile(bool IsSingleTarget, AMOBACharacter* CharacterTarget = NULL, FVector Direction = FVector(0,0,0), float InMaxDistance = 0.0f);

	// Handler for when projectile overlaps something
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
};
