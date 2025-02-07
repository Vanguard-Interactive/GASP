// Fill out your copyright notice in the Description page of Project Settings.

#include "Components/GASPCharacterMovementComponent.h"
#include "Curves/CurveVector.h"
#include "GameFramework/Character.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GASPCharacterMovementComponent)

UGASPCharacterMovementComponent::UGASPCharacterMovementComponent()
{
	bRunPhysicsWithNoController = true;
	bNetworkAlwaysReplicateTransformUpdateTimestamp = true;

	RotationRate = FRotator(0.f, -1.f, 0.f);
	BrakingFrictionFactor = 1.f;
	BrakingFriction = 0.f;
	BrakingDecelerationWalking = 1500.f;
	GroundFriction = 5.f;
	MinAnalogWalkSpeed = 150.f;
	MaxWalkSpeed = 500.f;
	MaxAcceleration = 800.f;
	PerchRadiusThreshold = 20.0f;
	bUseFlatBaseForFloorChecks = true;
	bUseSeparateBrakingFriction = true;
	bCanWalkOffLedgesWhenCrouching = true;

	NavAgentProps.bCanCrouch = true;

	NavMovementProperties.bUseAccelerationForPaths = true;
	SetCrouchedHalfHeight(60.f);
}

void UGASPCharacterMovementComponent::FGASPCharacterNetworkMoveData::ClientFillNetworkMoveData(
	const FSavedMove_Character& Move, ENetworkMoveType MoveType)
{
	Super::ClientFillNetworkMoveData(Move, MoveType);

	const FGASPSavedMove& SavedMove{StaticCast<const FGASPSavedMove&>(Move)};

	SavedRotationMode = SavedMove.SavedRotationMode;
	SavedGait = SavedMove.SavedGait;
}

bool UGASPCharacterMovementComponent::FGASPSavedMove::CanCombineWith(const FSavedMovePtr& NewMove,
                                                                     ACharacter* InCharacter, float MaxDelta) const
{
	const FGASPSavedMove* NewCombineMove{StaticCast<FGASPSavedMove*>(NewMove.Get())};

	return SavedRotationMode == NewCombineMove->SavedRotationMode && SavedGait == NewCombineMove->SavedGait &&
		Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UGASPCharacterMovementComponent::FGASPSavedMove::CombineWith(const FSavedMove_Character* OldMove,
                                                                  ACharacter* InCharacter, APlayerController* PC,
                                                                  const FVector& OldStartLocation)
{
	const auto OriginalRotation{OldMove->StartRotation};
	const auto OriginalRelativeRotation{OldMove->StartAttachRelativeRotation};

	const auto* NewUpdatedComponent{InCharacter->GetCharacterMovement()->UpdatedComponent.Get()};

	auto* MutablePreviousMove{const_cast<FSavedMove_Character*>(OldMove)};

	MutablePreviousMove->StartRotation = NewUpdatedComponent->GetComponentRotation();
	MutablePreviousMove->StartAttachRelativeRotation = NewUpdatedComponent->GetRelativeRotation();

	Super::CombineWith(OldMove, InCharacter, PC, OldStartLocation);

	MutablePreviousMove->StartRotation = OriginalRotation;
	MutablePreviousMove->StartAttachRelativeRotation = OriginalRelativeRotation;
}

void UGASPCharacterMovementComponent::FGASPSavedMove::Clear()
{
	Super::Clear();

	SavedGait = EGait::Run;
	bSavedRotationModeUpdate = false;
	SavedRotationMode = ERotationMode::OrientToMovement;
}

uint8 UGASPCharacterMovementComponent::FGASPSavedMove::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();

	if (bSavedRotationModeUpdate)
	{
		Result |= FLAG_Custom_0;
	}

	return Result;
}

void UGASPCharacterMovementComponent::FGASPSavedMove::SetMoveFor(ACharacter* C, float InDeltaTime,
                                                                 const FVector& NewAccel,
                                                                 FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	const UGASPCharacterMovementComponent* CharacterMovement{
		Cast<UGASPCharacterMovementComponent>(C->GetCharacterMovement())
	};

	if (CharacterMovement)
	{
		SavedGait = CharacterMovement->SafeGait;
		bSavedRotationModeUpdate = CharacterMovement->bSafeRotationModeUpdate;
		SavedRotationMode = CharacterMovement->SafeRotationMode;
	}
}

void UGASPCharacterMovementComponent::FGASPSavedMove::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);

	UGASPCharacterMovementComponent* CharacterMovement{
		Cast<UGASPCharacterMovementComponent>(C->GetCharacterMovement())
	};

	CharacterMovement->SafeGait = SavedGait;
	CharacterMovement->SafeRotationMode = SavedRotationMode;
}

UGASPCharacterMovementComponent::FNetworkPredictionData_Client_Base::FNetworkPredictionData_Client_Base(
	const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement)
{
}

FSavedMovePtr UGASPCharacterMovementComponent::FNetworkPredictionData_Client_Base::AllocateNewMove()
{
	return MakeShared<FGASPSavedMove>();
}

FNetworkPredictionData_Client* UGASPCharacterMovementComponent::GetPredictionData_Client() const
{
	check(PawnOwner != nullptr);

	if (!ClientPredictionData)
	{
		UGASPCharacterMovementComponent* MutableThis = const_cast<ThisClass*>(this);

		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_Base(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}

	return ClientPredictionData;
}

void UGASPCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	bSafeRotationModeUpdate = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}

void UGASPCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode,
                                                            uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if (IsMovingOnGround() || IsFalling())
	{
		RotationRate = FRotator(0.f, IsFalling() ? InAirRotationYaw : -1.f, 0.f);
	}
}

void UGASPCharacterMovementComponent::PhysicsRotation(float DeltaTime)
{
	if (HasAnimRootMotion())
	{
		return;
	}
	Super::PhysicsRotation(DeltaTime);
}

void UGASPCharacterMovementComponent::PhysNavWalking(float deltaTime, int32 Iterations)
{
	if (GaitSettings.GetMovementCurve())
	{
		GroundFriction = GaitSettings.GetMovementCurve()->GetVectorValue(GetMappedSpeed()).Z;
	}

	if (bSafeRotationModeUpdate)
	{
		UpdateRotationMode();
		bSafeRotationModeUpdate = false;
	}

	float& SpeedToUpdate{IsCrouching() ? MaxWalkSpeedCrouched : MaxWalkSpeed};
	SpeedToUpdate = GaitSettings.GetSpeed(SafeGait, Velocity, GetLastUpdateRotation(), IsCrouching());

	Super::PhysNavWalking(deltaTime, Iterations);
}

void UGASPCharacterMovementComponent::PhysWalking(float deltaTime, int32 Iterations)
{
	if (GaitSettings.GetMovementCurve())
	{
		GroundFriction = GaitSettings.GetMovementCurve()->GetVectorValue(GetMappedSpeed()).Z;
	}

	if (bSafeRotationModeUpdate)
	{
		UpdateRotationMode();
		bSafeRotationModeUpdate = false;
	}

	float& SpeedToUpdate{IsCrouching() ? MaxWalkSpeedCrouched : MaxWalkSpeed};
	SpeedToUpdate = GaitSettings.GetSpeed(SafeGait, Velocity, GetLastUpdateRotation(), IsCrouching());

	Super::PhysWalking(deltaTime, Iterations);
}

void UGASPCharacterMovementComponent::MoveSmooth(const FVector& InVelocity, const float DeltaSeconds,
                                                 FStepDownResult* OutStepDownResult)
{
	if (bSafeRotationModeUpdate)
	{
		UpdateRotationMode();
		bSafeRotationModeUpdate = false;
	}

	float& SpeedToUpdate{IsCrouching() ? MaxWalkSpeedCrouched : MaxWalkSpeed};
	SpeedToUpdate = GaitSettings.GetSpeed(SafeGait, Velocity, GetLastUpdateRotation(), IsCrouching());

	Super::MoveSmooth(InVelocity, DeltaSeconds, OutStepDownResult);
}

bool UGASPCharacterMovementComponent::HasMovementInputVector() const
{
	return !GetPendingInputVector().IsZero();
}

void UGASPCharacterMovementComponent::UpdateRotationMode()
{
	switch (SafeRotationMode)
	{
	case ERotationMode::OrientToMovement:
		bUseControllerDesiredRotation = false;
		bOrientRotationToMovement = true;
		break;
	default:
		bUseControllerDesiredRotation = true;
		bOrientRotationToMovement = false;
		break;
	}
}

void UGASPCharacterMovementComponent::Server_SetGait_Implementation(const EGait NewGait)
{
	SafeGait = NewGait;
}

void UGASPCharacterMovementComponent::SetGait(const EGait NewGait)
{
	if (SafeGait == NewGait)
	{
		return;
	}

	SafeGait = NewGait;
	if (GetCharacterOwner()->GetLocalRole() == ROLE_AutonomousProxy)
	{
		Server_SetGait(NewGait);
	}
}

void UGASPCharacterMovementComponent::SetRotationMode(const ERotationMode NewRotationMode)
{
	if (SafeRotationMode == NewRotationMode)
	{
		return;
	}
	if (PawnOwner->IsLocallyControlled())
	{
		SafeRotationMode = NewRotationMode;
		if (GetCharacterOwner()->GetLocalRole() == ROLE_AutonomousProxy)
		{
			Server_SetRotationMode(NewRotationMode);
		}
		bSafeRotationModeUpdate = true;
		return;
	}
	if (!PawnOwner->HasAuthority())
	{
		UpdateRotationMode();
	}
}

void UGASPCharacterMovementComponent::Server_SetRotationMode_Implementation(const ERotationMode NewRotationMode)
{
	SafeRotationMode = NewRotationMode;
}

float UGASPCharacterMovementComponent::GetMaxAcceleration() const
{
	if (!IsMovingOnGround() || !IsValid(GaitSettings.GetMovementCurve()))
	{
		return Super::GetMaxAcceleration();
	}

	return GaitSettings.GetMovementCurve()->GetVectorValue(GetMappedSpeed()).X;
}

float UGASPCharacterMovementComponent::GetMaxBrakingDeceleration() const
{
	if (!IsMovingOnGround() || !IsValid(GaitSettings.GetMovementCurve()))
	{
		return Super::GetMaxBrakingDeceleration();
	}

	return GaitSettings.GetMovementCurve()->GetVectorValue(GetMappedSpeed()).Y;
}

float UGASPCharacterMovementComponent::GetMappedSpeed() const
{
	float WalkSpeed = GaitSettings.GetSpeed(EGait::Walk, Velocity, GetLastUpdateRotation());
	float RunSpeed = GaitSettings.GetSpeed(EGait::Run, Velocity, GetLastUpdateRotation());
	float SprintSpeed = GaitSettings.GetSpeed(EGait::Sprint, Velocity, GetLastUpdateRotation());

	const auto Speed{UE_REAL_TO_FLOAT(Velocity.Size2D())};

	if (IsCrouching())
	{
		float CrouchedSpeed = GaitSettings.GetSpeed(SafeGait, Velocity, GetLastUpdateRotation(), IsCrouching());
		return FMath::GetMappedRangeValueClamped<float, float>({0.f, CrouchedSpeed}, {0.0f, 1.0f}, Speed);
	}

	if (Speed > RunSpeed)
	{
		return FMath::GetMappedRangeValueClamped<float, float>({RunSpeed, SprintSpeed}, {2.0f, 3.0f}, Speed);
	}
	if (Speed > WalkSpeed)
	{
		return FMath::GetMappedRangeValueClamped<float, float>({WalkSpeed, RunSpeed}, {1.0f, 2.0f}, Speed);
	}

	return FMath::GetMappedRangeValueClamped<float, float>({0.0f, WalkSpeed}, {0.0f, 1.0f}, Speed);
}
