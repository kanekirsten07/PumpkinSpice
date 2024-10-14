// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

class UTextBlock;

UCLASS()
class PUMPKINSPICE_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DisplayText;

	void SetDisplayText(const FString Text);

	UFUNCTION(BlueprintCallable)
	void ShowPlayerNetRole(APawn* Pawn);


protected:
	virtual void NativeDestruct() override;
};
