// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "MOBAPlayerController.h"

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

	InputComponent->BindAction("RightClick", IE_Pressed, this, &AMOBAPlayerController::OnRightClickPressed);
	InputComponent->BindAction("RightClick", IE_Released, this, &AMOBAPlayerController::OnRightClickReleased);
	InputComponent->BindAction("LeftClick", IE_Pressed, this, &AMOBAPlayerController::OnLeftClickPressed);
	InputComponent->BindAction("LeftClick", IE_Released, this, &AMOBAPlayerController::OnLeftClickReleased);
	InputComponent->BindAction("Stop", IE_Pressed, this, &AMOBAPlayerController::Stop);
	InputComponent->BindAction("Attack", IE_Pressed, this, &AMOBAPlayerController::Attack);
}

void AMOBAPlayerController::MoveToMouseCursor()
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
				if (IsHostile(*Cast<AMOBACharacter>(OverlappedActor))) 
				{
					currentdistance = FVector::Dist2D(MyCharacter->GetActorLocation(), OverlappedActor->GetActorLocation());
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
					MyCharacter->bIsAttacking = true;
					MovementType = EMovementType::MoveToEnemyTarget;
				}
			}
			MyCharacter->CombatStatusChangeDelegate.Broadcast(MyCharacter->bIsAttacking, MyCharacter->bIsInCombat);
		}
		else
		{
			MyAIController->MoveToLocation(AttackTarget, 5.0f, true, true, false, false, 0, true);
		}
	}
	else MovementType = EMovementType::None;
}

void AMOBAPlayerController::MoveToFriendlyTarget() 
{
	if (MyCharacter->MyFollowTarget) 
	{
		MyAIController->MoveToLocation(MyCharacter->MyFollowTarget->GetActorLocation(), 5.0f, false, true, false, false, 0, true);
	}
	// If we don't have a target, nothing to move to. Stop calling this function.
	else MovementType = EMovementType::None;
}

void AMOBAPlayerController::MoveToEnemyTarget() 
{
	if (MyCharacter->MyEnemyTarget) 
	{
		FVector MyLocation = MyCharacter->GetActorLocation();
		FVector TargetLocation = MyCharacter->MyEnemyTarget->GetActorLocation();
		float distance = FVector::Dist2D(MyLocation, TargetLocation);
		// Move to the target if out of range
		if (distance > MyCharacter->AttributeSet->AttackRange.GetCurrentValue())
		{
			MyAIController->MoveToLocation(TargetLocation, 0.9*MyCharacter->AttributeSet->AttackRange.GetCurrentValue(), false, true, false, false, 0, true);
		}
		// We're in range, try to attack
		else 
		{
			MyCharacter->bIsAttacking = true;
			if (MyCharacter->GetBasicAttackCooldown() == 0) 
			{
				// Signal Character to Start Attack. BP_InitiateBasicAttack() called too frequently
			}
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
				if (IsHostile(*HitCharacter)) // Check if the target is hostile
				{
					// Yes, target is hostile. Set character to attack
					MyCharacter->bIsAttacking = true;
					MyCharacter->MyEnemyTarget = HitCharacter;
					MyCharacter->MyFollowTarget = NULL;
					MovementType = EMovementType::MoveToEnemyTarget;
					MyCharacter->CombatStatusChangeDelegate.Broadcast(MyCharacter->bIsAttacking, MyCharacter->bIsInCombat);
				}
				else
				{
					// No, target is friendly. Set to Follow
					MyCharacter->bIsAttacking = false;
					MyCharacter->MyEnemyTarget = NULL;
					MyCharacter->MyFollowTarget = HitCharacter;
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
			if (IsHostile(*Cast<AMOBACharacter>(HitResult.GetActor()))) 
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

void AMOBAPlayerController::Attack()
{
	if (MyCharacter)
	{
		bAttackPending = true;
		CurrentMouseCursor = EMouseCursor::Crosshairs;
	}
}

// Check and see if another character is hostile (should we allow attacks or abilities on this target)
bool AMOBAPlayerController::IsHostile(AMOBACharacter& TargetCharacter) 
{
	if (TargetCharacter.IsValidLowLevel() && MyCharacter->IsValidLowLevel()) 
	{
		if (MyCharacter->MyTeam != TargetCharacter.MyTeam && TargetCharacter.MyTeam != ETeam::NeutralFriendly) 
		{
			return true;
		}
		else return false;
	}
	else return false;
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