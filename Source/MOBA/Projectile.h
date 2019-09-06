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
	
	// Sphere collision component.
	UPROPERTY(VisibleDefaultsOnly, Category = Projectile)
		USphereComponent* CollisionComponent;

	// Projectile movement component.
	UPROPERTY(VisibleAnywhere, Category = Movement)
		UProjectileMovementComponent* ProjectileMovementComponent;

	UFUNCTION(BlueprintImplementableEvent)
		void OnTargetReached(AMOBACharacter* InTarget);

	UFUNCTION(BlueprintCallable)
		void InitializeProjectile(AMOBACharacter* Target);

	// Handler for when projectile overlaps something
	UFUNCTION()
	void OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult);
};
