// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Kismet/KismetMathLibrary.h"

AProjectile::AProjectile()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Use a sphere as a simple collision representation.
	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	// Set collision component to use default projectile collision settings.
	CollisionComponent->BodyInstance.SetCollisionProfileName(TEXT("Projectile"));
	// Set the sphere's collision radius.
	CollisionComponent->InitSphereRadius(15.0f);
	// Set the root component to be the collision component.
	RootComponent = CollisionComponent;
	// Bind to Overlap Delegate
	CollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AProjectile::OnOverlap);
	
	// Use this component to drive this projectile's movement.
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->SetUpdatedComponent(CollisionComponent);
	ProjectileMovementComponent->InitialSpeed = 1500.0f;
	ProjectileMovementComponent->MaxSpeed = 1500.0f;
	ProjectileMovementComponent->Velocity = FVector{ 0,0,0 };
	ProjectileMovementComponent->bIsHomingProjectile = false;
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bShouldBounce = false;
	ProjectileMovementComponent->ProjectileGravityScale = 0;
	ProjectileMovementComponent->Friction = 0;
	InitialLifeSpan = 0.0f; // Projectile lives until it hits its target.
	TargetLocation = FVector{ 0,0,0 };
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	SpawnedLocation = GetActorLocation();
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (TargetLocation != FVector{ 0,0,0 }) 
	{
		// We have a set distance, check if we reached it
		if (FVector::Dist2D(SpawnedLocation,GetActorLocation()) >= MaxDistance) 
		{
			Destroy();
		}
	}	
}

// Update the Destination based on the target's current position
void AProjectile::InitializeProjectile(bool IsSingleTarget, AMOBACharacter* CharacterTarget, FVector Direction, float InMaxDistance)
{
	// If Target is specified, then the projectile is a homing projectile
	if (CharacterTarget)
	{
		ProjectileMovementComponent->bIsHomingProjectile = true;
		ProjectileMovementComponent->HomingAccelerationMagnitude = 10000; // Instantly hit max speed
		ProjectileMovementComponent->HomingTargetComponent = CharacterTarget->ProjectileTarget;
		bIsSingleTarget = IsSingleTarget;
		MyEnemyTarget = CharacterTarget;
		bIsInitialized = true;
	}
	// If no target, then moving in a direction. Direction is either infinite or defined.
	else if (Direction != FVector{0, 0, 0})
	{
		bIsSingleTarget = IsSingleTarget;
		SetActorRotation(Direction.Rotation());
		ProjectileMovementComponent->Velocity = ProjectileMovementComponent->InitialSpeed * Direction;
		// If a Distance to travel is specified.
		if (InMaxDistance > 0) 
		{
			TargetLocation = Direction.UpVector * InMaxDistance;
			MaxDistance = InMaxDistance;
		}
		bIsInitialized = true;
	}
	else Destroy();
}

void AProjectile::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) 
{
	if (bIsInitialized) 
	{
		if (this->MyEnemyTarget)
		{
			// Check if the overlapped component belongs to the enemy target
			AMOBACharacter* othercharacter = Cast<AMOBACharacter>(OtherActor);
			if (othercharacter == this->MyEnemyTarget)
			{
				// We made it to the target, check if its the mesh 
				if (othercharacter->GetMesh() == Cast<USkeletalMeshComponent>(OtherComp))
				{
					// broadcast a delegate and destroy ourself
					this->OnTargetReached(this->MyEnemyTarget);
					this->Destroy();
				}

			}
		}
	}
}