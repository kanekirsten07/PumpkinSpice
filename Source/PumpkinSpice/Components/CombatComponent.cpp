// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "PumpkinSpice/Characters/PumpkinSpiceCharacter.h"
#include "PumpkinSpice/Weapon/Weapon.h"
#include "Kismet/GameplayStatics.h"
#include "PumpkinSpice/PlayerController/PumpkinSpicePlayerController.h"
#include "PumpkinSpice/HUD/PumpkinSpiceHUD.h"

// Sets default values for this component's properties
UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);

	BaseWalkSpeed = 500.f;
	AimWalkSpeed = 250.f;
	SprintSpeed = 1000.f;
}

//////////////////////////////////////////////////////////////////////////
void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming)
	DOREPLIFETIME(UCombatComponent, bSprinting)
	DOREPLIFETIME(UCombatComponent, bDancing)
}

//////////////////////////////////////////////////////////////////////////
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	}
}

//////////////////////////////////////////////////////////////////////////
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SetHUDCrosshairs(DeltaTime);
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;

	PlayerController = PlayerController == nullptr ? Cast<APumpkinSpicePlayerController>(Character->Controller) : PlayerController;
	if (PlayerController)
	{
		HUD = HUD == nullptr ? Cast<APumpkinSpiceHUD>(PlayerController->GetHUD()) : HUD;
		FHUDPackage HUDPackage;
		if (EquippedWeapon)
		{
			HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
			HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
			HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
			HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
			HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
		}
		else
		{
			
			HUDPackage.CrosshairsCenter = nullptr;
			HUDPackage.CrosshairsLeft = nullptr;
			HUDPackage.CrosshairsRight = nullptr;
			HUDPackage.CrosshairsTop = nullptr;
			HUDPackage.CrosshairsBottom = nullptr;
		}
		HUD->SetHUDPackage(HUDPackage);
	}
}

//////////////////////////////////////////////////////////////////////////
void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	ServerSetAiming(bIsAiming);

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

//////////////////////////////////////////////////////////////////////////
void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
}

void UCombatComponent::SetSprinting(bool bIsSprinting)
{
	bSprinting = bIsSprinting;
	ServerSetSprinting(bIsSprinting);

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsSprinting ? SprintSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::ServerSetSprinting_Implementation(bool bIsSprinting)
{
	bSprinting = bIsSprinting;
}

void UCombatComponent::SetDancing(bool bIsDancing)
{
	bDancing = bIsDancing;
	ServerSetDancing(bIsDancing);
}

void UCombatComponent::ServerSetDancing_Implementation(bool bIsDancing)
{
	bDancing = bIsDancing;
}

//////////////////////////////////////////////////////////////////////////
void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed;

	if (bFireButtonPressed)
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		ServerFire(HitResult.ImpactPoint);
	}
}

void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (Character)
	{
		Character->PlayFireMontage();
		if (EquippedWeapon)
		{
			EquippedWeapon->Fire(false, TraceHitTarget);
		}
	}
}

void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(UGameplayStatics::GetPlayerController(this, 0), CrosshairLocation, CrosshairWorldPosition, CrosshairWorldDirection);

	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;

		FVector End = Start + CrosshairWorldDirection * 80000.f;

		GetWorld()->LineTraceSingleByChannel(TraceHitResult, Start, End, ECollisionChannel::ECC_Visibility);
		// Debug code
		/*if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
		}
		else
		{
			DrawDebugSphere(GetWorld(), TraceHitResult.ImpactPoint, 12.f, 12, FColor::Red);
		}*/
	}
}

//////////////////////////////////////////////////////////////////////////
void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (!Character || !WeaponToEquip)
	{
		return;
	}

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));

	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}

	EquippedWeapon->SetOwner(Character);
}

