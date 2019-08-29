// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "MOBAGameMode.h"
#include "MOBAPlayerController.h"
#include "MOBACharacter.h"
#include "UObject/ConstructorHelpers.h"

AMOBAGameMode::AMOBAGameMode()
{
	// use our custom PlayerController class
	PlayerControllerClass = AMOBAPlayerController::StaticClass();
}