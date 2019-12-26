// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "MOBAPlayerController.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimInstance.h"
#include "Engine/LocalPlayer.h"

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
		AttackCommandActive = false;
		break;
	case EMovementType::MoveToFriendlyTarget: MoveToFriendlyTarget();
		AttackCommandActive = false;
		break;
	case EMovementType::MoveToEnemyTarget: MoveToEnemyTarget();
		break;
	case EMovementType::MoveToAttackLocation: MoveToAttackLocation(AttackLocation);
		AttackCommandActive = false;
		break;
	default: AttackCommandActive = false;  break;
	}

	// Check if we should scroll the camera with our mouse
	if (!bFollowPlayerCharacter) 
	{
		CalculateMouseScroll();
		MoveCamera();
	}
	// No, lock to MyCharacter
	else 
	{
		if (MyCharacter && MyCamera) 
		{
			MyCamera->SetActorLocation(MyCharacter->GetActorLocation());
		}
	}
}

void AMOBAPlayerController::BeginPlay()
{
	MyCamera = GetPawn();
	if (!MyCamera)
	{
		UE_LOG(LogTemp, Verbose, TEXT("UMyClass %s BeginPlay error! No possessed pawn."), *GetNameSafe(this));
	}
	CurrentMouseCursor = DefaultMouseCursor;
	if (MyCharacter && MyCamera) 
	{
		MyCamera->SetActorLocation(MyCharacter->GetActorLocation());
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
	InputComponent->BindAction("ToggleCameraLock", IE_Pressed, this, &AMOBAPlayerController::ToggleCameraLock);
	InputComponent->BindAction("CenterCamera", IE_Pressed, this, &AMOBAPlayerController::ToggleCameraLock);
	InputComponent->BindAction("CenterCamera", IE_Released, this, &AMOBAPlayerController::ToggleCameraLock);
	InputComponent->BindAction("MoveCameraForward", IE_Pressed, this, &AMOBAPlayerController::MoveCameraForward);
	InputComponent->BindAction("MoveCameraBackward", IE_Pressed, this, &AMOBAPlayerController::MoveCameraBackward);
	InputComponent->BindAction("MoveCameraLeft", IE_Pressed, this, &AMOBAPlayerController::MoveCameraLeft);
	InputComponent->BindAction("MoveCameraRight", IE_Pressed, this, &AMOBAPlayerController::MoveCameraRight);
	InputComponent->BindAction("MoveCameraForward", IE_Released, this, &AMOBAPlayerController::MoveCameraForwardReleased);
	InputComponent->BindAction("MoveCameraBackward", IE_Released, this, &AMOBAPlayerController::MoveCameraBackwardReleased);
	InputComponent->BindAction("MoveCameraLeft", IE_Released, this, &AMOBAPlayerController::MoveCameraLeftReleased);
	InputComponent->BindAction("MoveCameraRight", IE_Released, this, &AMOBAPlayerController::MoveCameraRightReleased);
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
			StopMontage();
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
				}
			}
			if (MyCharacter->MyEnemyTarget)
			{
				MyCharacter->bIsAttacking = true;
				MovementType = EMovementType::MoveToEnemyTarget;
			}
		}
		else
		{
			StopMontage();
			MyAIController->MoveToLocation(AttackTarget, 10.0f, true, true, false, false, 0, true);
		}
	}
	else 
	{
		MovementType = EMovementType::None;
	}
}

void AMOBAPlayerController::MoveToFriendlyTarget() 
{
	if (MyCharacter->MyFollowTarget && MyAIController) 
	{
		StopMontage();
		MyAIController->MoveToLocation(MyCharacter->MyFollowTarget->GetActorLocation(), 5.0f, false, true, false, false, 0, true);
	}
	// If we don't have a target, nothing to move to. Stop calling this function.
	else 
	{
		MovementType = EMovementType::None;
		AttackCommandActive = false;
	}
}

void AMOBAPlayerController::MoveToEnemyTarget() 
{
	if (MyCharacter->MyEnemyTarget && MyAIController) 
	{
		if (!AttackCommandActive) 
		{
			MyCharacter->TryBasicAttack();
			AttackCommandActive = true;
		}
	}
	// If we don't have a target, nothing to move to. Stop calling this function.
	else 
	{
		MovementType = EMovementType::None;
	}
}

void AMOBAPlayerController::StopMontage() 
{
	if (MyCharacter) 
	{
		UAnimMontage* currentmontage = MyCharacter->GetCurrentMontage();
		MyCharacter->StopAnimMontage(currentmontage);
	}
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
					StopMontage();
					MovementType = EMovementType::MoveToEnemyTarget;
					MyCharacter->CombatStatusChangeDelegate.Broadcast(MyCharacter->bIsAttacking, MyCharacter->bIsInCombat);
				}
				else
				{
					// No, target is friendly. Set to Follow
					MyCharacter->bIsAttacking = false;
					MyCharacter->MyEnemyTarget = NULL;
					MyCharacter->MyFollowTarget = HitCharacter;
					StopMontage();
					MovementType = EMovementType::MoveToFriendlyTarget;
				}
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
					MyCharacter->MyFollowTarget = NULL;
					MyCharacter->CombatStatusChangeDelegate.Broadcast(MyCharacter->bIsAttacking, MyCharacter->bIsInCombat);
					// set flag to keep updating destination until released
					StopMontage();
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
		AMOBACharacter* HitCharacter = Cast<AMOBACharacter>(HitResult.GetActor());
		if (HitCharacter) 
		{
			if (MyCharacter->IsHostile(HitCharacter))
			{
				MyCharacter->bIsAttacking = true;
				MyCharacter->MyEnemyTarget = HitCharacter;
				MyCharacter->MyFocusTarget = HitCharacter;
				MyCharacter->MyFollowTarget = NULL;
				MovementType = EMovementType::MoveToEnemyTarget;
			}
			else 
			{
				MyCharacter->MyFollowTarget = HitCharacter;
				MyCharacter->MyFocusTarget = HitCharacter;
				MyCharacter->MyEnemyTarget = NULL;
				MyCharacter->bIsAttacking = false;
				MyCharacter->CombatStatusChangeDelegate.Broadcast(MyCharacter->bIsAttacking, MyCharacter->bIsInCombat);
				MovementType = EMovementType::MoveToFriendlyTarget;
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
		else 
		{
			MyCharacter->MyFocusTarget = NULL;
		}
	}
}

void AMOBAPlayerController::OnLeftClickReleased()
{
	
}

// Mouse Camera Control when camera is not locked
void AMOBAPlayerController::CalculateMouseScroll() 
{
	float X;
	float Y;
	int32 SizeX;
	int32 SizeY;
	GetMousePosition(X, Y);
	GetViewportSize(SizeX, SizeY);
	if (X >= (SizeX * 0.9f)) 
	{
		MouseY = 1.0f;
	}
	else if (X <= (SizeX * 0.1f) && X > 0.0f && Y > 0.0f && Y < SizeY) 
	{
		MouseY = -1.0f;
	}
	else MouseY = 0.0f;
	if (Y >= (SizeY * 0.9f)) 
	{
		MouseX = -1.0f;
	}
	else if (Y <= (SizeY * 0.1f) && Y > 0.0f && X > 0.0f && X < SizeX)
	{
		MouseX = 1.0f;
	}
	else MouseX = 0.0f;
}
// Move Camera when not locked
void AMOBAPlayerController::MoveCamera() 
{
	if (MyCamera) 
	{
		float x = (MouseX * MouseScrollSpeed) + ((CameraForward + CameraBackward) * KeyboardScrollSpeed);
		float y = (MouseY * MouseScrollSpeed) + ((CameraLeft + CameraRight) * KeyboardScrollSpeed);
		FVector movement = FVector{x, y, 0.0f};
		MyCamera->AddMovementInput(movement);
	}
}

// Event handler for user pressing the "Stop" input action
void AMOBAPlayerController::Stop() 
{
	FVector location;
	if (MyCharacter) 
	{
		if (MyAIController) 
		{
			MyAIController->StopMovement();
		}
		MyCharacter->bIsAttacking = false;
		MyCharacter->MyEnemyTarget = NULL;
		MyCharacter->MyFollowTarget = NULL;
		AttackCommandActive = false;
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

// Toggle whether the camera follows the pawn or not
void AMOBAPlayerController::ToggleCameraLock() 
{
	bFollowPlayerCharacter = bFollowPlayerCharacter ? false : true;
}

// Keyboard camera control when camera is not locked
void AMOBAPlayerController::MoveCameraForward() 
{
	CameraForward = 1.0f;
}
void AMOBAPlayerController::MoveCameraForwardReleased() 
{
	CameraForward = 0.0f;
}
void AMOBAPlayerController::MoveCameraBackward() 
{
	CameraBackward = -1.0f;
}
void AMOBAPlayerController::MoveCameraBackwardReleased() 
{
	CameraBackward = 0.0f;
}
void AMOBAPlayerController::MoveCameraLeft() 
{
	CameraLeft = -1.0f;
}
void AMOBAPlayerController::MoveCameraLeftReleased() 
{
	CameraLeft = 0.0f;
}
void AMOBAPlayerController::MoveCameraRight() 
{
	CameraRight = 1.0f;
}
void AMOBAPlayerController::MoveCameraRightReleased() 
{
	CameraRight = 0.0f;
}