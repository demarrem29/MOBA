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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	APawn* MyPawn;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AMOBACharacter* MyCharacter;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	AAIController* MyAIController;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
		ETeam MyTeam;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		EMovementType MovementType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
		FVector AttackLocation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Targeting")
		USphereComponent* AttackCollisionSphere;

protected:	
	// Begin PlayerController interface
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	// End PlayerController interface


	/** Movement Functions. */
	void MoveToMouseCursor();
	void MoveToAttackLocation(FVector AttackTarget);
	void MoveToFriendlyTarget();
	void MoveToEnemyTarget();

	/** Input handlers for mouse action. */
	void OnRightClickPressed();
	void OnRightClickReleased();
	void OnLeftClickPressed();
	void OnLeftClickReleased();

	/*  Input handlers for keyboard action. */
	void Stop();
	void Attack();

	// Attack Input bools
	bool bAttackPending;
	bool IsHostile(AMOBACharacter& TargetCharacter);

	virtual void BeginPlay() override;
};


