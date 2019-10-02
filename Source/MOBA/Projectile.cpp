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
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bShouldBounce = false;
	InitialLifeSpan = 0.0f; // Projectile lives until it hits its target.
	TargetLocation = FVector{ 0,0,0 };
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();	
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (bIsHoming && bIsInitialized)
	{ 
		if (MyEnemyTarget)
		{
			InitializeProjectile(bIsSingleTarget, MyEnemyTarget);
		}
	}
	if (TargetLocation != FVector{ 0,0,0 }) 
	{
		// We have a set distance, check if we reached it
		if (TargetLocation.UpVector == GetActorLocation().UpVector) 
		{
			Destroy();
		}
	}
	
}

// Update the Destination based on the target's current position
void AProjectile::InitializeProjectile(bool IsSingleTarget, AMOBACharacter* CharacterTarget, FVector Direction, float MaxDistance)
{
	// If Target is specified, then the projectile is a homing projectile
	if (CharacterTarget)
	{
		bIsHoming = true;
		bIsSingleTarget = IsSingleTarget;
		MyEnemyTarget = CharacterTarget;
		USceneComponent* ProjectileTarget = CharacterTarget->GetRootComponent();
		ProjectileMovementComponent->Velocity = ProjectileMovementComponent->InitialSpeed * (UKismetMathLibrary::FindLookAtRotation(this->GetActorLocation(), ProjectileTarget->GetSocketLocation((TEXT("neck_01"))))).Vector() * FVector { 1, 1, 0 };
		bIsInitialized = true;
	}
	// If no target, then moving in a direction. Direction is either infinite or defined.
	else if (Direction != FVector{0, 0, 0})
	{
		bIsHoming = false;
		bIsSingleTarget = IsSingleTarget;
		SetActorRotation(Direction.Rotation());
		ProjectileMovementComponent->Velocity = ProjectileMovementComponent->InitialSpeed * Direction;
		// If a Distance to travel is specified.
		if (MaxDistance > 0) 
		{
			TargetLocation = Direction.UpVector * MaxDistance;
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