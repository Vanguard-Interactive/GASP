// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/GASPCharacter.h"
#include "Components/GASPCharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AGASPCharacter::AGASPCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UGASPCharacterMovementComponent>(CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	bReplicates = true;
	
	bUseControllerRotationRoll = bUseControllerRotationPitch = bUseControllerRotationYaw = false;
}

// Called when the game starts or when spawned
void AGASPCharacter::BeginPlay()
{
	Super::BeginPlay();

	SetGait(EGait::Run, true);
	SetRotationMode(ERotationMode::OrientToMovement, true);
}

// Called every frame
void AGASPCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	SetMovementState(GetVelocity().Size2D() > 0.f 
		? EMovementState::Moving 
		: EMovementState::Idle);
}

void AGASPCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	SetReplicateMovement(true);
}

void AGASPCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, Gait);
	DOREPLIFETIME(ThisClass, RotationMode);
	DOREPLIFETIME(ThisClass, MovementMode);
	DOREPLIFETIME(ThisClass, MovementState);
}

void AGASPCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PrevCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PrevCustomMode);

	if (GetBCharacterMovement()->IsMovingOnGround())
	{
		SetMovementMode(ECMovementMode::OnGround);
	}
	else if (GetBCharacterMovement()->IsFalling())
	{
		SetMovementMode(ECMovementMode::InAir);
	}
}

void AGASPCharacter::SetGait(EGait NewGait, bool bForce)
{
	if (NewGait != Gait || bForce)
	{
		Gait = NewGait;
		GetBCharacterMovement()->SetGait(NewGait);
		
		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetGait(NewGait);
		}
	}
}

void AGASPCharacter::SetRotationMode(ERotationMode NewRotationMode, bool bForce)
{
	if (NewRotationMode != RotationMode || bForce)
	{
		RotationMode = NewRotationMode;
		GetBCharacterMovement()->SetRotationMode(NewRotationMode);

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetRotationMode(NewRotationMode);
		}
	}
}

void AGASPCharacter::SetMovementMode(ECMovementMode NewMovementMode, bool bForce)
{
	if (NewMovementMode != MovementMode || bForce)
	{
		MovementMode = NewMovementMode;

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetMovementMode(NewMovementMode);
		}
	}
}

void AGASPCharacter::Server_SetMovementMode_Implementation(ECMovementMode NewMovementMode)
{
	MovementMode = NewMovementMode;
}

void AGASPCharacter::Server_SetRotationMode_Implementation(ERotationMode NewRotationMode)
{
	RotationMode = NewRotationMode;
}

void AGASPCharacter::Server_SetGait_Implementation(EGait NewGait)
{
	Gait = NewGait;
}

void AGASPCharacter::SetMovementState(EMovementState NewMovementState, bool bForce)
{
	if (NewMovementState != MovementState || bForce)
	{
		MovementState = NewMovementState;

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetMovementState(NewMovementState);
		}
	}
}

void AGASPCharacter::Server_SetMovementState_Implementation(EMovementState NewMovementState)
{
	MovementState = NewMovementState;
}

void AGASPCharacter::MoveAction(const FVector2D& Value)
{
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation = FRotator(0.f, Rotation.Yaw, 0.f);

	const FVector ForwardDirectionDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirectionDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirectionDirection, Value.X);
	AddMovementInput(RightDirectionDirection, Value.Y);
}

void AGASPCharacter::LookAction(const FVector2D& Value)
{
	AddControllerYawInput(Value.X);
	AddControllerPitchInput(-1 * Value.Y);
}

UGASPCharacterMovementComponent* AGASPCharacter::GetBCharacterMovement() const
{
	return StaticCast<UGASPCharacterMovementComponent*>(GetCharacterMovement());
}
