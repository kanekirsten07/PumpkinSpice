// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "PumpkinSpiceGameMode.generated.h"

UCLASS(minimalapi)
class APumpkinSpiceGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	APumpkinSpiceGameMode();

	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
};



