// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "MOBAPlayerController.h"
#include "Components/CapsuleComponent.h"

AMOBAPlayerController::AMOBAPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Hand;
	CurrentMouseCursor = EMouseCursor::Hand;
	MyTeam = ETeam::BottomSide;
	MovementType = EMovementType::None;
	// Create a sphere component and place it at AttackTarget
	AttackCollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Attack Move Radius"), false);
	AttackCollisionSphere->SetSphereRadius(1000.0f);
}

void AMOBAPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// keep updating the destination every tick while desired
	switch (MovementType)
	{
	case EMovementType::None: break;
	case EMovementType::MoveToCursor: MoveToMouseCursor(); 
		break;
	case EMovementType::MoveToEnemyTarget: MoveToEnemyTarget();
		break;
	case EMovementType::MoveToFriendlyTarget: MoveToFriendlyTarget();
		break;
	case EMovementType::MoveToAttackLocation: MoveToAttackLocation(AttackLocation); 
		break;
	default: break;
	}
}

void AMOBAPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();
	// Mouse Bindings
	InputComponent->BindAction("RightClick", IE_Pressed, this, &AMOBAPlayerController::OnRightClickPressed);
	InputComponent->BindAction("RightClick", IE_Released, this, &AMOBAPlayerController::OnRightClickReleased);
	InputComponent->BindAction("LeftClick", IE_Pressed, this, &AMOBAPlayerController::OnLeftClickPressed);
	InputComponent->BindAction("LeftClick", IE_Released, this, &AMOBAPlayerController::OnLeftClickReleased);
	
	// Default keyboard input "S" to stop moving and attacking
	InputComponent->BindAction("Stop", IE_Pressed, this, &AMOBAPlayerController::Stop);
	// Default keyboard input "A" to set a new target. Left clicking on the ground does attack move to location.
	// Left clicking on an enemy starts performing basic attacks on them. Right click to cancel.
	InputComponent->BindAction("Attack", IE_Pressed, this, &AMOBAPlayerController::Attack);
}

void AMOBAPlayerController::MoveToMouseCursor()
{
	if (MyCharacter && MyAIController) 
	{
		// Trace to see what is under the mouse cursor
		FHitResult Hit;
		GetHitResultUnderCursor(ECC_Visibility, false, Hit);
		if (Hit.bBlockingHit)
		{
			// We hit something, move there
 			UAIBlueprintHelperLibrary::SimpleMoveToLocation(MyAIController, Hit.ImpactPoint);
		}
	}
	
}

void AMOBAPlayerController::MoveToAttackLocation(FVector AttackTarget) 
{
	if (MyCharacter) 
	{
		// Array to store GetOverlappingActors return value 
		TArray<AActor*> OverlappedActors;
		// Move Attack Collision sphere to new location
		AttackCollisionSphere->SetWorldLocation(AttackTarget);
		// Check if there are enemies within the sphere
		AttackCollisionSphere->GetOverlappingActors(OverlappedActors, TSubclassOf<AMOBACharacter>());
		if (OverlappedActors.Num() > 0)
		{
			float minimumdistance = -1;
			float currentdistance;
			for (auto& OverlappedActor : OverlappedActors)
			{
				if (MyCharacter->IsHostile(Cast<AMOBACharacter>(OverlappedActor))) 
				{
					currentdistance = FVector::Dist2D(AttackCollisionSphere->GetComponentLocation(), OverlappedActor->GetActorLocation());
					if (minimumdistance == -1)
					{
						minimumdistance = currentdistance;
						MyCharacter->MyEnemyTarget = Cast<AMOBACharacter>(OverlappedActor);
					}
					else if (currentdistance < minimumdistance)
					{
						minimumdistance = currentdistance;
						MyCharacter->MyEnemyTarget = Cast<AMOBACharacter>(OverlappedActor);
					}
					MovementType = EMovementType::MoveToEnemyTarget;
				}
			}
		}
		else
		{
			MyAIController->MoveToLocation(AttackTarget, 10.0f, true, true, false, false, 0, true);
		}
	}
	else MovementType = EMovementType::None;
}

void AMOBAPlayerController::MoveToFriendlyTarget() 
{
	if (MyCharacter->MyFollowTarget && MyAIController) 
	{
		MyAIController->MoveToLocation(MyCharacter->MyFollowTarget->GetActorLocation(), 5.0f, false, true, false, false, 0, true);
	}
	// If we don't have a target, nothing to move to. Stop calling this function.
	else MovementType = EMovementType::None;
}

void AMOBAPlayerController::MoveToEnemyTarget() 
{
	if (MyCharacter->MyEnemyTarget && MyAIController) 
	{
		// Move to the target if out of range
		if (!MyCharacter->RangeDetector->IsOverlappingActor(MyCharacter->MyEnemyTarget))
		{
			MyAIController->MoveToLocation(MyCharacter->MyEnemyTarget->GetActorLocation(), MyCharacter->AttributeSet->AttackRange.GetCurrentValue(), true, true, false, false, 0, true);
		}
		// We're in range, try to attack
		else 
		{
			MyAIController->StopMovement();
			MyCharacter->bIsAttacking = true;
			MyCharacter->TryBasicAttack();
		}
	}
	// If we don't have a target, nothing to move to. Stop calling this function.
	else MovementType = EMovementType::None;
}

void AMOBAPlayerController::OnRightClickPressed()
{
	if (bAttackPending) 
	{
		bAttackPending = false;
		CurrentMouseCursor = DefaultMouseCursor;
	}
	else 
	{
		// Trace to see what is under the mouse cursor
		FHitResult HitResult;
		// Try to get a pawn first
		GetHitResultUnderCursor(ECC_Pawn, false, HitResult);
		if (Cast<AMOBACharacter>(HitResult.GetActor()))
		{
			// Check if the target hit was an attackable target
			AActor* HitActor = HitResult.GetActor();
			AMOBACharacter* HitCharacter = Cast<AMOBACharacter>(HitActor);
			if (!MyCharacter)
			{
				UE_LOG(LogTemp, Error, TEXT("Couldn't get a pointer to a pawn. No possessed pawn, or the pawn is the wrong class!"));
				return;
			}
			if (HitCharacter)
			{
				ETeam SourceTeam = MyCharacter->MyTeam;
				ETeam TargetTeam = HitCharacter->MyTeam;
				if (MyCharacter->IsHostile(HitCharacter)) // Check if the target is hostile
				{
					// Yes, target is hostile. Set character to attack
					MyCharacter->bIsAttacking = true;
					MyCharacter->MyEnemyTarget = HitCharacter;
					MyCharacter->MyFollowTarget = NULL;
					UAnimMontage* currentmontage = MyCharacter->GetCurrentMontage();
					MyCharacter->StopAnimMontage(currentmontage);
					MovementType = EMovementType::MoveToEnemyTarget;
					MyCharacter->CombatStatusChangeDelegate.Broadcast(MyCharacter->bIsAttacking, MyCharacter->bIsInCombat);
				}
				else
				{
					// No, target is friendly. Set to Follow
					MyCharacter->bIsAttacking = false;
					MyCharacter->MyEnemyTarget = NULL;
					MyCharacter->MyFollowTarget = HitCharacter;
					UAnimMontage* currentmontage = MyCharacter->GetCurrentMontage();
					MyCharacter->StopAnimMontage(currentmontage);
					MovementType = EMovementType::MoveToFriendlyTarget;
				}
				// Change rotation to look at target. Only use yaw.
				FRotator myrotation = MyCharacter->GetActorRotation();
				FRotator lookatrotation = UKismetMathLibrary::FindLookAtRotation(MyCharacter->GetActorLocation(), HitCharacter->GetActorLocation());
				myrotation.SetComponentForAxis(EAxis::Z, lookatrotation.Yaw);
				MyCharacter->SetActorRotation(myrotation, ETeleportType::None);
			}
		}
		else // Didn't find a pawn target, so move to location instead
		{
			GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
			if (HitResult.bBlockingHit)
			{
				if (MyCharacter)
				{
					// No target, turn off auto-attacks.
					MyCharacter->bIsAttacking = false;
					MyCharacter->MyEnemyTarget = NULL;
					MyCharacter->CombatStatusChangeDelegate.Broadcast(MyCharacter->bIsAttacking, MyCharacter->bIsInCombat);
					// set flag to keep updating destination until released
					UAnimMontage* currentmontage = MyCharacter->GetCurrentMontage();
					MyCharacter->StopAnimMontage(currentmontage);
					MovementType = EMovementType::MoveToCursor;
				}
			}
		}
	}
}

void AMOBAPlayerController::OnRightClickReleased()
{
	// clear flag to indicate we should stop updating the destination
	if (MovementType == EMovementType::MoveToCursor) 
	{
		MovementType = EMovementType::None;
	}
}

void AMOBAPlayerController::OnLeftClickPressed()
{
	// Trace to see what is under the mouse cursor
	FHitResult HitResult;
	GetHitResultUnderCursor(ECC_Pawn, false, HitResult);
	if (bAttackPending)
	{
		if (Cast<AMOBACharacter>(HitResult.GetActor())) 
		{
			if (MyCharacter->IsHostile(Cast<AMOBACharacter>(HitResult.GetActor()))) 
			{
				MyCharacter->MyEnemyTarget = Cast<AMOBACharacter>(HitResult.GetActor());
				MovementType = EMovementType::MoveToEnemyTarget;
			}
		}
		else
		{
			AttackLocation = HitResult.ImpactPoint;
			MovementType = EMovementType::MoveToAttackLocation;
		}
		CurrentMouseCursor = DefaultMouseCursor;
		bAttackPending = false;
	}
	else 
	{
		if (Cast<AMOBACharacter>(HitResult.GetActor())) 
		{
			MyCharacter->MyFocusTarget = Cast<AMOBACharacter>(HitResult.GetActor());
		}
	}
}

void AMOBAPlayerController::OnLeftClickReleased()
{
	
}

// Event handler for user pressing the "Stop" input action
void AMOBAPlayerController::Stop() 
{
	FVector location;
	if (MyCharacter) 
	{
		location = MyCharacter->GetActorLocation();
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(MyAIController, location);
		MyCharacter->bIsAttacking = false;
		MyCharacter->MyEnemyTarget = NULL;
		MyCharacter->MyFollowTarget = NULL;
		MyCharacter->CombatStatusChangeDelegate.Broadcast(MyCharacter->bIsAttacking, MyCharacter->bIsInCombat);
	}
	// clear flag to indicate we should stop updating the destination
	MovementType = EMovementType::None;
}

// Event handler for user pressing the "Attack" input action
void AMOBAPlayerController::Attack()
{
	if (MyCharacter)
	{
		bAttackPending = true;
		CurrentMouseCursor = EMouseCursor::Crosshairs;
	}
}

void AMOBAPlayerController::BeginPlay() 
{
	MyPawn = GetPawn();
	if (!MyPawn) 
	{
		UE_LOG(LogTemp, Verbose, TEXT("UMyClass %s BeginPlay error! No possessed pawn."), *GetNameSafe(this));
	}
	CurrentMouseCursor = DefaultMouseCursor;
}