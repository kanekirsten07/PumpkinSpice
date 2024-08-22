// Copyright Epic Games, Inc. All Rights Reserved.

#include "PumpkinSpiceGameMode.h"
#include "PumpkinSpiceCharacter.h"
#include "UObject/ConstructorHelpers.h"

APumpkinSpiceGameMode::APumpkinSpiceGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
