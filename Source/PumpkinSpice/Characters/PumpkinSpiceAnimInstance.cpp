// Fill out your copyright notice in the Description page of Project Settings.


#include "PumpkinSpiceAnimInstance.h"
#include "PumpkinSpiceCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UPumpkinSpiceAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	PSCharacter = Cast<APumpkinSpiceCharacter>(TryGetPawnOwner());
}

void UPumpkinSpiceAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (PSCharacter == nullptr)
	{
		PSCharacter = Cast<APumpkinSpiceCharacter>(TryGetPawnOwner());
	}

	if (PSCharacter == nullptr) return;

	FVector Velocity = PSCharacter->GetVelocity();
	Velocity.Z = 0.f;

	Speed = Velocity.Size();

	bIsInAir = PSCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = PSCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
}
