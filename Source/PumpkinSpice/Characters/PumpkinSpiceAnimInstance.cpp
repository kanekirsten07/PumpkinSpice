// Fill out your copyright notice in the Description page of Project Settings.


#include "PumpkinSpiceAnimInstance.h"
#include "Kismet/KismetMathLibrary.h"
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

	bWeaponEquipped = PSCharacter->IsWeaponEquipped();
	bIsCrouched = PSCharacter->bIsCrouched;
	bAiming = PSCharacter->IsAiming();

	//Offset yaw for strafing
	FRotator AimRotation = PSCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(PSCharacter->GetVelocity());
	YawOffset = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;

	AO_Pitch = PSCharacter->GetAimOffsetPitch();
	
	//UE_LOG(LogTemp, Log, TEXT("Yaw Offset %f"), YawOffset);
	
}
