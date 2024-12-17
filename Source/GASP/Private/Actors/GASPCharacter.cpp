// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/GASPCharacter.h"
#include "Components/GASPCharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

// Sets default values
AGASPCharacter::AGASPCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer.SetDefaultSubobjectClass<UGASPCharacterMovementComponent>(CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	bUseControllerRotationRoll = bUseControllerRotationPitch = bUseControllerRotationYaw = false;

	// bAlwaysRelevant = true;
	bReplicates = true;
    SetReplicatingMovement(true);
	GetMesh()->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;
}

// Called when the game starts or when spawned
void AGASPCharacter::BeginPlay()
{
	Super::BeginPlay();
	GetMesh()->AddTickPrerequisiteActor(this);

	UnCrouch();
	SetGait(DesiredGait, true);
	SetRotationMode(DesiredRotationMode, true);
}

// Called every frame
void AGASPCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AGASPCharacter::PostRegisterAllComponents()
{
	Super::PostRegisterAllComponents();
	
	MovementComponent = Cast<UGASPCharacterMovementComponent>(GetCharacterMovement());
}

void AGASPCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	SetStanceMode(EStanceMode::Crouch);
}

void AGASPCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
	Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

	SetStanceMode(EStanceMode::Stand);
}

void AGASPCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
	MovementComponent = Cast<UGASPCharacterMovementComponent>(GetCharacterMovement());
}

void AGASPCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams Parameters;
	Parameters.bIsPushBased = true;
	Parameters.Condition = COND_SkipOwner;
	
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, Gait, Parameters);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, RotationMode, Parameters);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MovementMode, Parameters);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, MovementState, Parameters);
	DOREPLIFETIME_WITH_PARAMS_FAST(ThisClass, StanceMode, Parameters);
}

void AGASPCharacter::OnMovementModeChanged(EMovementMode PrevMovementMode, uint8 PrevCustomMode)
{
	Super::OnMovementModeChanged(PrevMovementMode, PrevCustomMode);
	
	if (MovementComponent->IsMovingOnGround())
	{
		SetMovementMode(ECMovementMode::OnGround);
	}
	else if (MovementComponent->IsFalling())
	{
		SetMovementMode(ECMovementMode::InAir);
	}
}

void AGASPCharacter::SetGait(const EGait NewGait, const bool bForce)
{
	if (NewGait != Gait || bForce)
	{
		const auto OldGait{Gait};
		Gait = NewGait;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, Gait, this);
		
		MovementComponent->SetGait(NewGait);
		
		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetGait(NewGait);
		}
		OnGaitChanged(OldGait);
	}
}

void AGASPCharacter::SetRotationMode(const ERotationMode NewRotationMode, const bool bForce)
{	
	if (NewRotationMode != RotationMode || bForce)
	{
		const auto OldRotationMode{RotationMode};
		RotationMode = NewRotationMode;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, RotationMode, this);
		
		MovementComponent->SetRotationMode(NewRotationMode);

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetRotationMode(NewRotationMode);
		}
		
		OnRotationModeChanged(OldRotationMode);
	}
}

void AGASPCharacter::SetMovementMode(const ECMovementMode NewMovementMode, const bool bForce)
{
	if (NewMovementMode != MovementMode || bForce)
	{
		MovementMode = NewMovementMode;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MovementMode, this);
		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetMovementMode(NewMovementMode);
		}
	}
}

void AGASPCharacter::Server_SetMovementMode_Implementation(const ECMovementMode NewMovementMode)
{
	MovementMode = NewMovementMode;
}

void AGASPCharacter::Server_SetRotationMode_Implementation(const ERotationMode NewRotationMode)
{
	SetRotationMode(NewRotationMode);
}

void AGASPCharacter::Server_SetGait_Implementation(const EGait NewGait)
{
	SetGait(NewGait);
}

void AGASPCharacter::SetMovementState(const EMovementState NewMovementState, const bool bForce)
{
	if (NewMovementState != MovementState || bForce)
	{
		const auto OldMovementState{MovementState};
		MovementState = NewMovementState;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, MovementState, this);
		
		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetMovementState(NewMovementState);
		}

		OnMovementStateChanged(OldMovementState);
	}
}

void AGASPCharacter::SetStanceMode(EStanceMode NewStanceMode, bool bForce)
{
	if (StanceMode != NewStanceMode || bForce)
	{
		const auto OldStanceMode{StanceMode};
		StanceMode = NewStanceMode;
		MARK_PROPERTY_DIRTY_FROM_NAME(ThisClass, StanceMode, this);

		UE_LOG(LogTemp, Warning, TEXT("Crouch"))

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetStanceMode(NewStanceMode);
		}

		OnStanceModeChanged(OldStanceMode);
	}
}

void AGASPCharacter::Server_SetStanceMode_Implementation(const EStanceMode NewStanceMode)
{
	SetStanceMode(NewStanceMode);
}

void AGASPCharacter::Server_SetMovementState_Implementation(const EMovementState NewMovementState)
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

bool AGASPCharacter::CanSprint()
{
	if (RotationMode == ERotationMode::OrientToMovement) return true;

	const FVector Acceleration{ IsLocallyControlled() ? GetPendingMovementInputVector() :
		MovementComponent->GetCurrentAcceleration() };
	const FRotator Rotation{ GetActorRotation() - Acceleration.ToOrientationRotator() };
	
	return FMath::Abs(Rotation.Yaw) < 50.f;
}

UGASPCharacterMovementComponent* AGASPCharacter::GetBCharacterMovement() const
{
	return StaticCast<UGASPCharacterMovementComponent*>(GetCharacterMovement());
}
