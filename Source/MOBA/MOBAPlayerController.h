// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Runtime/Engine/Classes/Components/DecalComponent.h"
#include "MOBACharacter.h"
#include "MOBAAttributeSet.h"
#include "Engine/World.h"
#include "Kismet/KismetMathLibrary.h"
#include "AIModule/Classes/AIController.h"
#include "MOBAPlayerController.generated.h"

UENUM(BlueprintType)
enum class EMovementType : uint8
{
	None				 UMETA(DisplayName = "Movement Disabled"),
	MoveToCursor		 UMETA(DisplayName = "Move To Mouse Cursor"),
	MoveToEnemyTarget	 UMETA(DisplayName = "Move To Attack Range Against a Specific Target"),
	MoveToFriendlyTarget UMETA(DisplayName = "Follow an Allied Unit"),
	MoveToAttackLocation UMETA(Display Name = "Acquire an Attack Target Near a Location")
};

UCLASS()
class AMOBAPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMOBAPlayerController();

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Pawn")
		AMOBACharacter* MyCharacter;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Pawn")
		AAIController* MyAIController;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
		ETeam MyTeam;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
		APawn* MyCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
		bool bFollowPlayerCharacter = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
		float KeyboardScrollSpeed = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
		float MouseScrollSpeed = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		float MouseX = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		float MouseY = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		float CameraForward = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		float CameraBackward = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		float CameraLeft = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
		float CameraRight = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
		EMovementType MovementType;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
		FVector AttackLocation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Targeting")
		USphereComponent* AttackCollisionSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Targeting")
		bool AttackCommandActive;

protected:	
	// Begin PlayerController interface
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	// End PlayerController interface

	//Camera Functions
	void MoveCamera();

	/** Movement Functions. */
	void MoveToMouseCursor();
	void MoveToAttackLocation(FVector AttackTarget);
	void MoveToFriendlyTarget();
	void MoveToEnemyTarget();
	void StopMontage();

	/** Input handlers for mouse action. */
	void OnRightClickPressed();
	void OnRightClickReleased();
	void OnLeftClickPressed();
	void OnLeftClickReleased();
	void CalculateMouseScroll();

	/*  Input handlers for keyboard action. */
	void Stop();
	void Attack();
	void ToggleCameraLock();
	void MoveCameraForward();
	void MoveCameraForwardReleased();
	void MoveCameraBackward();
	void MoveCameraBackwardReleased();
	void MoveCameraLeft();
	void MoveCameraLeftReleased();
	void MoveCameraRight();
	void MoveCameraRightReleased();

	// Attack Input bools
	bool bAttackPending;
	
	virtual void BeginPlay() override;
};


