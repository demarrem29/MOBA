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
	// Set collision component to generate overlap events every time it overlaps something.
	CollisionComponent->SetGenerateOverlapEvents(true);
	CollisionComponent->bMultiBodyOverlap = 1;
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
	InitializeProjectile(MyEnemyTarget);
}

void AProjectile::InitializeProjectile(AMOBACharacter* Target) 
{
	if (Target)
	{
		MyEnemyTarget = Target;
		ProjectileMovementComponent->Velocity = ProjectileMovementComponent->InitialSpeed * UKismetMathLibrary::FindLookAtRotation(this->GetActorLocation(), MyEnemyTarget->GetActorLocation()).Vector();
	}
}

void AProjectile::OnOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) 
{
	// Get a reference to self
	AProjectile* self = Cast<AProjectile>(OverlappedComponent->GetOwner());
	if (self) 
	{
		if (self->MyEnemyTarget) 
		{
			// Check if the overlapped component belongs to the enemy target
			AMOBACharacter* othercharacter = Cast<AMOBACharacter>(OtherActor);
			if (othercharacter) 
			{
				// We made it to the target, broadcast a delegate and destroy ourself
				if (othercharacter->GetRootComponent() == OtherComp) 
				{
					self->OnTargetReached(self->MyEnemyTarget);
					self->Destroy();
				}
				
			}
		}
	}

}