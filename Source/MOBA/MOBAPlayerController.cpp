// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "MOBAPlayerController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Runtime/Engine/Classes/Components/DecalComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "MOBACharacter.h"
#include "MOBAAttributeSet.h"
#include "Engine/World.h"

AMOBAPlayerController::AMOBAPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

void AMOBAPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	// keep updating the destination every tick while desired
	if (bMoveToMouseCursor)
	{
		MoveToMouseCursor();
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

	// support touch devices 
	InputComponent->BindTouch(EInputEvent::IE_Pressed, this, &AMOBAPlayerController::MoveToTouchLocation);
	InputComponent->BindTouch(EInputEvent::IE_Repeat, this, &AMOBAPlayerController::MoveToTouchLocation);
}

void AMOBAPlayerController::MoveToMouseCursor()
{
	if (UHeadMountedDisplayFunctionLibrary::IsHeadMountedDisplayEnabled())
	{
		if (AMOBACharacter* MyPawn = Cast<AMOBACharacter>(GetPawn()))
		{
			if (MyPawn->GetCursorToWorld())
			{
				UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, MyPawn->GetCursorToWorld()->GetComponentLocation());
			}
		}
	}
	else
	{
		// Trace to see what is under the mouse cursor
		FHitResult Hit;
		GetHitResultUnderCursor(ECC_Visibility, false, Hit);

		if (Hit.bBlockingHit)
		{
			// We hit something, move there
			SetNewMoveDestination(Hit.ImpactPoint);
		}
	}
}

void AMOBAPlayerController::MoveToTouchLocation(const ETouchIndex::Type FingerIndex, const FVector Location)
{
	FVector2D ScreenSpaceLocation(Location);

	// Trace to see what is under the touch location
	FHitResult HitResult;
	GetHitResultAtScreenPosition(ScreenSpaceLocation, CurrentClickTraceChannel, true, HitResult);
	if (HitResult.bBlockingHit)
	{
		// We hit something, move there
		SetNewMoveDestination(HitResult.ImpactPoint);
	}
}

void AMOBAPlayerController::SetNewMoveDestination(const FVector DestLocation)
{
	APawn* const MyPawn = GetPawn();
	if (MyPawn)
	{
		FVector MyLocation = MyPawn->GetActorLocation();
		FVector TargetLocation = DestLocation;
		AMOBACharacter* MyCharacter = Cast<AMOBACharacter>(MyPawn);
		if (MyCharacter) 
		{
			if (MyCharacter->bIsAttacking) // If we're trying to perform an auto attack, we only want to move to the attack range distance from the target
			{
				// Construct a destination vector, using the targets z location
				float x = DestLocation.X - MyLocation.X;
				float y = DestLocation.Y - MyLocation.Y;
				float hypotenuse = FMath::Sqrt((x*x + y * y)); 
				float newhypotenuse = hypotenuse - MyCharacter->AttributeSet->AttackRange.GetCurrentValue();
				float newx = ((x*newhypotenuse) / hypotenuse) + MyLocation.X;
				float newy = ((y*newhypotenuse) / hypotenuse) + MyLocation.Y;
				TargetLocation.X = newx;
				TargetLocation.Y = newy;
			}
		}
		// Move Command
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, TargetLocation);
	}
}

void AMOBAPlayerController::OnRightClickPressed()
{
	// Trace to see what is under the mouse cursor
	FHitResult HitResult;
	AMOBACharacter* MyCharacter = Cast<AMOBACharacter>(GetPawn());
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
			ETeam MyTeam = MyCharacter->MyTeam;
			ETeam TargetTeam = HitCharacter->MyTeam;
			if (MyTeam != TargetTeam && TargetTeam != ETeam::NeutralFriendly) // Check if the target is hostile
			{
				// Yes, target is hostile. Set character to attack
				MyCharacter->bIsAttacking = true;
				MyCharacter->MyEnemyTarget = HitCharacter;
			}
		}
	}
	else // Didn't find a pawn target, so move to location instead
	{
		GetHitResultUnderCursor(ECC_Visibility, false, HitResult);
		if (HitResult.bBlockingHit) 
		{
			if (MyCharacter)
			// Yes, target is hostile. Set character to attack
			MyCharacter->bIsAttacking = false;
			MyCharacter->MyEnemyTarget = NULL;
		}
	}
	
	// set flag to keep updating destination until released
	bMoveToMouseCursor = true;
}

void AMOBAPlayerController::OnRightClickReleased()
{
	// clear flag to indicate we should stop updating the destination
	bMoveToMouseCursor = false;
}

void AMOBAPlayerController::OnLeftClickPressed()
{
	
}

void AMOBAPlayerController::OnLeftClickReleased()
{
	
}

void AMOBAPlayerController::Stop() 
{
	APawn* const MyPawn = GetPawn();
	AMOBACharacter* MyCharacter;
	FVector location;
	if (MyPawn) 
	{
		location = MyPawn->GetActorLocation();
		SetNewMoveDestination(location);
		MyCharacter = Cast<AMOBACharacter>(MyPawn);
		if (MyCharacter) 
		{
			MyCharacter->bIsAttacking = false;
			MyCharacter->MyEnemyTarget = NULL;
		}
	}
	// clear flag to indicate we should stop updating the destination
	bMoveToMouseCursor = false;
}