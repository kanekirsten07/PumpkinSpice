// Copyright Epic Games, Inc. All Rights Reserved.

#include "PumpkinSpiceCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "PumpkinSpice/Weapon/Weapon.h"
#include "PumpkinSpice/Components/CombatComponent.h"
#include "Kismet/KismetMathLibrary.h"

DEFINE_LOG_CATEGORY(LogPumpkinSpiceCharacter);

//////////////////////////////////////////////////////////////////////////
// APumpkinSpiceCharacter

APumpkinSpiceCharacter::APumpkinSpiceCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

void APumpkinSpiceCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (CombatComponent)
	{
		CombatComponent->Character = this;
	}
}

//////////////////////////////////////////////////////////////////////////
void APumpkinSpiceCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(APumpkinSpiceCharacter, OverlappingWeapon, COND_OwnerOnly);
}

//////////////////////////////////////////////////////////////////////////
void APumpkinSpiceCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	AimOffset(DeltaTime);
}

//////////////////////////////////////////////////////////////////////////
void APumpkinSpiceCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
}

//////////////////////////////////////////////////////////////////////////
void APumpkinSpiceCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}

	OverlappingWeapon = Weapon;

	if (IsLocallyControlled())
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
bool APumpkinSpiceCharacter::IsWeaponEquipped()
{
	return (CombatComponent && CombatComponent->EquippedWeapon);
}

//////////////////////////////////////////////////////////////////////////
bool APumpkinSpiceCharacter::IsAiming()
{
	return (CombatComponent && CombatComponent->bAiming);
}

bool APumpkinSpiceCharacter::IsDancing()
{
	// :< 
	return false;
}

//////////////////////////////////////////////////////////////////////////
AWeapon* APumpkinSpiceCharacter::GetEquippedWeapon()
{
	if (CombatComponent)
	{
		return CombatComponent->EquippedWeapon;
	}

	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
void APumpkinSpiceCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}

	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

//////////////////////////////////////////////////////////////////////////
void APumpkinSpiceCharacter::ServerEquipButtonPressed_Implementation()
{
	if (CombatComponent)
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void APumpkinSpiceCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// Jumping or Dodge, duck, dip, dive, and dodge....ing
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &APumpkinSpiceCharacter::JumpOrDodge);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APumpkinSpiceCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APumpkinSpiceCharacter::Look);

		// Equipping
		EnhancedInputComponent->BindAction(EquipAction, ETriggerEvent::Triggered, this, &APumpkinSpiceCharacter::Equip);

		//Crouching
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Started, this, &APumpkinSpiceCharacter::OnCrouchPressed);
		EnhancedInputComponent->BindAction(CrouchAction, ETriggerEvent::Completed, this, &APumpkinSpiceCharacter::OnCrouchPressed);

		//Sprint
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &APumpkinSpiceCharacter::OnSprintPressed);
		EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &APumpkinSpiceCharacter::OnSprintPressed);

		//Aiming
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Started, this, &APumpkinSpiceCharacter::OnAimPressed);
		EnhancedInputComponent->BindAction(AimAction, ETriggerEvent::Completed, this, &APumpkinSpiceCharacter::OnAimReleased);

		//Got me Dancing
		EnhancedInputComponent->BindAction(DanceAction, ETriggerEvent::Triggered, this, &APumpkinSpiceCharacter::Dance);
	}
	else
	{
		UE_LOG(LogPumpkinSpiceCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void APumpkinSpiceCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		auto foo = ForwardDirection * MovementVector.Y;

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		auto bar = RightDirection * MovementVector.X;

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void APumpkinSpiceCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APumpkinSpiceCharacter::Equip(const FInputActionValue& Value)
{
	if (CombatComponent)
	{
		if (HasAuthority())
		{
			CombatComponent->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			ServerEquipButtonPressed();
		}
	}
}

void APumpkinSpiceCharacter::OnCrouchPressed(const FInputActionValue& Value)
{
	if (bIsCrouched)
	{
		Super::UnCrouch(false);
	}
	else
	{
		Super::Crouch(false);
	}
}

void APumpkinSpiceCharacter::OnSprintPressed(const FInputActionValue& Value)
{

}

void APumpkinSpiceCharacter::Dance(const FInputActionValue& Value)
{

}

void APumpkinSpiceCharacter::OnAimPressed()
{
	if (CombatComponent)
	{
		CombatComponent->SetAiming(true);
		GetCharacterMovement()->bOrientRotationToMovement = false;
		bUseControllerRotationYaw = true;
	}
}

void APumpkinSpiceCharacter::OnAimReleased()
{
	if (CombatComponent)
	{
		CombatComponent->SetAiming(false);
		GetCharacterMovement()->bOrientRotationToMovement = true;
		bUseControllerRotationYaw = false;
	}
}

void APumpkinSpiceCharacter::AimOffset(float DeltaTime)
{
	if (CombatComponent && CombatComponent->EquippedWeapon == nullptr)
	{
		return;
	}

	if (!IsAiming())
	{
		AO_Pitch = 0.f;
	}

	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;

	float Speed = Velocity.Size();

	FRotator CurrentAimRotation = FRotator(GetBaseAimRotation().Pitch, 0.F, 0.f);
	AO_Pitch = CurrentAimRotation.Pitch;
	
	// correct for netcode clamping on server-controlled actors
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);

		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void APumpkinSpiceCharacter::JumpOrDodge(const FInputActionValue& Value)
{
	UCharacterMovementComponent* MovementComponent = GetCharacterMovement();
	if (!MovementComponent)
	{
		return;
	}

	bool bIsMoving = MovementComponent->GetCurrentAcceleration().Size() > 0.f ? true : false;
	
	if (IsWeaponEquipped())
	{
		//Start Dodging base off of direction being held
		FVector MovementVector = GetActorRotation().Vector();
		int MovementOffset = 1;

		if (!IsAiming())
		{
			// Start dodging based off of character actor rotation
			MovementVector = GetActorRotation().Vector();
			// if we're not moving, amplify movement
			MovementOffset = 1000;
		}
		else
		{
			MovementVector = MovementComponent->GetCurrentAcceleration();
		}
		
		TSharedPtr<FRootMotionSource_ConstantForce> ConstantForce = MakeShared<FRootMotionSource_ConstantForce>();
		ConstantForce->AccumulateMode = ERootMotionAccumulateMode::Additive;
		ConstantForce->Priority = 5;
		ConstantForce->Force = MovementVector * MovementOffset;
		ConstantForce->Duration = 0.1f;
		ConstantForce->FinishVelocityParams.Mode = ERootMotionFinishVelocityMode::MaintainLastRootMotionVelocity;
		ConstantForce->FinishVelocityParams.SetVelocity = FVector(0.0, 0.0, 0.0);
		ConstantForce->FinishVelocityParams.ClampVelocity = 0.f;
		
		//Dodge
		MovementComponent->ApplyRootMotionSource(ConstantForce);
	}
}
